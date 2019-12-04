#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "client.h"
#include "player.h"
#include "trains.h"
#include "vehicle.h"
#include "nodes.h"
#include "weapons.h"
#include "soundent.h"
#include "monsters.h"
#include "shake.h"
#include "decals.h"
#include "gamerules.h"
#include "training_gamerules.h"
#include "game.h"
#include "hltv.h"
#include "pm_shared.h"
#include "studio.h"
#include "trigger.h"
#include "buildable.h"
#include <string>

extern DLL_GLOBAL BOOL g_fGameOver;
extern DLL_GLOBAL BOOL g_fDrawLines;
extern DLL_GLOBAL int g_iSkillLevel, gDisplayTitle;
extern DLL_GLOBAL int g_fIsTraceLine;
DLL_GLOBAL short g_sModelIndexPlayerClass[9];

int gEvilImpulse101;
//char g_szMapBriefingText[512];
BOOL gInitHUD = TRUE;

extern void CopyToBodyQue(entvars_t *pev);
extern void respawn(entvars_t *pev, BOOL fCopyCorpse);
extern Vector VecBModelOrigin(entvars_t *pevBModel);
extern edict_t *EntSelectSpawnPoint(CBaseEntity *pPlayer);
extern CGraph WorldGraph;

#define FLASH_DRAIN_TIME 1.2
#define FLASH_CHARGE_TIME 0.2

TYPEDESCRIPTION CBasePlayer::m_playerSaveData[] =
{
	DEFINE_FIELD(CBasePlayer, m_afButtonLast, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_afButtonPressed, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_afButtonReleased, FIELD_INTEGER),
	DEFINE_ARRAY(CBasePlayer, m_rgItems, FIELD_INTEGER, MAX_ITEMS),
	DEFINE_FIELD(CBasePlayer, m_afPhysicsFlags, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_flTimeStepSound, FIELD_TIME),
	DEFINE_FIELD(CBasePlayer, m_flTimeWeaponIdle, FIELD_TIME),
	DEFINE_FIELD(CBasePlayer, m_flSwimTime, FIELD_TIME),
	DEFINE_FIELD(CBasePlayer, m_flDuckTime, FIELD_TIME),
	DEFINE_FIELD(CBasePlayer, m_flWallJumpTime, FIELD_TIME),
	DEFINE_FIELD(CBasePlayer, m_flSuitUpdate, FIELD_TIME),
	DEFINE_ARRAY(CBasePlayer, m_rgSuitPlayList, FIELD_INTEGER, CSUITPLAYLIST),
	DEFINE_FIELD(CBasePlayer, m_iSuitPlayNext, FIELD_INTEGER),
	DEFINE_ARRAY(CBasePlayer, m_rgiSuitNoRepeat, FIELD_INTEGER, CSUITNOREPEAT),
	DEFINE_ARRAY(CBasePlayer, m_rgflSuitNoRepeatTime, FIELD_TIME, CSUITNOREPEAT),
	DEFINE_FIELD(CBasePlayer, m_lastDamageAmount, FIELD_INTEGER),
	DEFINE_ARRAY(CBasePlayer, m_rgpPlayerItems, FIELD_CLASSPTR, MAX_ITEM_TYPES),
	DEFINE_FIELD(CBasePlayer, m_pActiveItem, FIELD_CLASSPTR),
	DEFINE_FIELD(CBasePlayer, m_pLastItem, FIELD_CLASSPTR),
	DEFINE_ARRAY(CBasePlayer, m_rgAmmo, FIELD_INTEGER, MAX_AMMO_SLOTS),
	DEFINE_FIELD(CBasePlayer, m_idrowndmg, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_idrownrestored, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_tSneaking, FIELD_TIME),
	DEFINE_FIELD(CBasePlayer, m_iTrain, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_bitsHUDDamage, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_flFallVelocity, FIELD_FLOAT),
	DEFINE_FIELD(CBasePlayer, m_iTargetVolume, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_iWeaponVolume, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_iExtraSoundTypes, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_iWeaponFlash, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_fInitHUD, FIELD_BOOLEAN),
	DEFINE_FIELD(CBasePlayer, m_tbdPrev, FIELD_TIME),
	DEFINE_FIELD(CBasePlayer, m_pTank, FIELD_EHANDLE),
	DEFINE_FIELD(CBasePlayer, m_iHideHUD, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_iFOV, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_flDisplayHistory, FIELD_INTEGER),
	DEFINE_FIELD(CBasePlayer, m_iJoiningState, FIELD_INTEGER),
};

int giPrecacheGrunt = 0;
int gmsgCurWeapon = 0;
int gmsgGeigerRange = 0;
int gmsgHealth = 0;
int gmsgDamage = 0;
int gmsgBattery = 0;
int gmsgTrain = 0;
int gmsgLogo = 0;
int gmsgHudText = 0;
int gmsgSayText = 0;
int gmsgTextMsg = 0;
int gmsgWeaponList = 0;
int gmsgResetHUD = 0;
int gmsgInitHUD = 0;
int gmsgViewMode = 0;
int gmsgCheckModels = 0;
int gmsgShowGameTitle = 0;
int gmsgDeathMsg = 0;
int gmsgScoreAttrib = 0;
int gmsgScoreInfo = 0;
int gmsgTeamInfo = 0;
int gmsgTeamScore = 0;
int gmsgGameMode = 0;
int gmsgMOTD = 0;
int gmsgServerName = 0;
int gmsgAmmoPickup = 0;
int gmsgWeapPickup = 0;
int gmsgItemPickup = 0;
int gmsgHideWeapon = 0;
int gmsgSetFOV = 0;
int gmsgShowMenu = 0;
int gmsgShake = 0;
int gmsgFade = 0;
int gmsgAmmoX = 0;
int gmsgSendAudio = 0;
int gmsgRoundTime = 0;
int gmsgMoney = 0;
int gmsgArmorType = 0;
int gmsgBlinkAcct = 0;
int gmsgStatusValue = 0;
int gmsgStatusText = 0;
int gmsgStatusIcon = 0;
int gmsgBarTime = 0;
int gmsgReloadSound = 0;
int gmsgCrosshair = 0;
int gmsgNVGToggle = 0;
int gmsgRadar = 0;
int gmsgSpectator = 0;
int gmsgVGUIMenu = 0;
int gmsgAllowSpec = 0;
int gmsgBombDrop = 0;
int gmsgBombPickup = 0;
int gmsgSendCorpse = 0;
int gmsgMGUIMenu = 0;
int gmsgMGUIPrint = 0;
int gmsgHudMenu = 0;
int gmsgDrawFX = 0;
int gmsgRespawnTime = 0;
int gmsgWeaponAnimEx = 0;
int gmsgFlameIgnite = 0;
int gmsgObjectMsg = 0;
int gmsgCPState = 0;
int gmsgCPInit = 0;
int gmsgBuildDeath = 0;//build deathmsg
int gmsgDisguise = 0;
int gmsgRTInit = 0;
int gmsgRTState = 0;
int gmsgHUDStatus = 0;
int gmsgHUDBuild = 0;
int gmsgStatsInfo = 0;
int gmsgTimeLimit = 0;
int gmsgDominate = 0;
int gmsgRevenge = 0;
int gmsgWeaponInfo = 0;
int gmsgMapObject = 0;
int gmsgMetaRender = 0;
int gmsgSpawnInit = 0;
int gmsgPlayerVars = 0;

//cs16
int gmsgSpecHealth = 0;
int gmsgSpecHealth2 = 0;
int gmsgForceCam = 0;
int gmsgFog = 0;
int gmsgHLTV = 0;
int gmsgADStop = 0;

void LinkUserMessages(void)
{
	if (gmsgCurWeapon)
		return;

	gmsgCurWeapon = REG_USER_MSG("CurWeapon", 3);
	gmsgGeigerRange = REG_USER_MSG("Geiger", 1);
	gmsgHealth = REG_USER_MSG("Health", 2);
	gmsgDamage = REG_USER_MSG("Damage", 12);
	gmsgBattery = REG_USER_MSG("Battery", 2);
	gmsgTrain = REG_USER_MSG("Train", 1);
	gmsgHudText = REG_USER_MSG("HudText", -1);
	gmsgSayText = REG_USER_MSG("SayText", -1);
	gmsgTextMsg = REG_USER_MSG("TextMsg", -1);
	gmsgWeaponList = REG_USER_MSG("WeaponList", -1);
	gmsgResetHUD = REG_USER_MSG("ResetHUD", 0);
	gmsgInitHUD = REG_USER_MSG("InitHUD", 0);
	gmsgViewMode = REG_USER_MSG("ViewMode", 0);
	//gmsgCheckModels = REG_USER_MSG("CheckModels", 0);//unused in CS
	gmsgShowGameTitle = REG_USER_MSG("GameTitle", 1);
	gmsgDeathMsg = REG_USER_MSG("DeathMsg", -1);
	gmsgScoreAttrib = REG_USER_MSG("ScoreAttrib", 3);
	gmsgScoreInfo = REG_USER_MSG("ScoreInfo", 6);
	gmsgTeamInfo = REG_USER_MSG("TeamInfo", -1);
	gmsgTeamScore = REG_USER_MSG("TeamScore", -1);
	gmsgGameMode = REG_USER_MSG("GameMode", 1);
	gmsgMOTD = REG_USER_MSG("MOTD", -1);
	gmsgServerName = REG_USER_MSG("ServerName", -1);
	gmsgAmmoPickup = REG_USER_MSG("AmmoPickup", 2);
	gmsgWeapPickup = REG_USER_MSG("WeapPickup", 1);
	gmsgItemPickup = REG_USER_MSG("ItemPickup", -1);
	gmsgHideWeapon = REG_USER_MSG("HideWeapon", 2);
	gmsgSetFOV = REG_USER_MSG("SetFOV", 1);
	gmsgShowMenu = REG_USER_MSG("ShowMenu", -1);
	gmsgShake = REG_USER_MSG("ScreenShake", sizeof(ScreenShake));
	gmsgFade = REG_USER_MSG("ScreenFade", sizeof(ScreenFade));
	gmsgAmmoX = REG_USER_MSG("AmmoX", 2);
	gmsgSendAudio = REG_USER_MSG("SendAudio", -1);
	gmsgRoundTime = REG_USER_MSG("RoundTime", 5);
	gmsgMoney = REG_USER_MSG("Money", 5);
	gmsgArmorType = REG_USER_MSG("ArmorType", 1);
	gmsgBlinkAcct = REG_USER_MSG("BlinkAcct", 1);
	gmsgStatusValue = REG_USER_MSG("StatusValue", -1);
	gmsgStatusText = REG_USER_MSG("StatusText", -1);
	gmsgStatusIcon = REG_USER_MSG("StatusIcon", -1);
	gmsgBarTime = REG_USER_MSG("BarTime", 2);
	gmsgReloadSound = REG_USER_MSG("ReloadSound", 2);
	gmsgCrosshair = REG_USER_MSG("Crosshair", 1);
	gmsgNVGToggle = REG_USER_MSG("NVGToggle", 1);
	gmsgRadar = REG_USER_MSG("Radar", 7);
	gmsgSpectator = REG_USER_MSG("Spectator", 2);
	gmsgVGUIMenu = REG_USER_MSG("VGUIMenu", -1);
	gmsgAllowSpec = REG_USER_MSG("AllowSpec", 1);
	gmsgBombDrop = REG_USER_MSG("BombDrop", 6);
	gmsgBombPickup = REG_USER_MSG("BombPickup", 0);
	gmsgSendCorpse = REG_USER_MSG("ClCorpse", -1);
	//cs16
	gmsgSpecHealth = REG_USER_MSG("SpecHealth", 1);
	gmsgSpecHealth2 = REG_USER_MSG("SpecHealth2", 2);
	gmsgForceCam = REG_USER_MSG("ForceCam", 3);
	gmsgFog = REG_USER_MSG("Fog", 7);
	gmsgHLTV = REG_USER_MSG("HLTV", 2);
	gmsgADStop = REG_USER_MSG("ADStop", 0);
	//ckf added
	gmsgMGUIMenu = REG_USER_MSG("MGUIMenu", 3);
	gmsgMGUIPrint = REG_USER_MSG("MGUIPrint", -1);
	gmsgHudMenu = REG_USER_MSG("HudMenu", 3);
	gmsgDrawFX = REG_USER_MSG("DrawFX", -1);
	gmsgRespawnTime = REG_USER_MSG("RespawnTime", 1);
	gmsgWeaponAnimEx = REG_USER_MSG("WpnAnimEx", 3);
	gmsgFlameIgnite = REG_USER_MSG("FlameIgnite", -1);
	gmsgObjectMsg = REG_USER_MSG("ObjectMsg", -1);
	gmsgCPState = REG_USER_MSG("CPState", 10);
	gmsgCPInit = REG_USER_MSG("CPInit", -1);
	gmsgBuildDeath = REG_USER_MSG("BuildDeath", -1);
	gmsgDisguise = REG_USER_MSG("Disguise", -1);
	gmsgRTInit = REG_USER_MSG("RTInit", -1);
	gmsgRTState = REG_USER_MSG("RTState", 8);
	//We have client-side hudstatus now
	//gmsgHUDStatus = REG_USER_MSG("HUDStatus", -1);
	gmsgHUDBuild = REG_USER_MSG("HUDBuild", -1);
	gmsgStatsInfo = REG_USER_MSG("StatsInfo", 3);
	gmsgTimeLimit = REG_USER_MSG("TimeLimit", 2);
	gmsgDominate = REG_USER_MSG("Dominate", -1);
	gmsgRevenge = REG_USER_MSG("Revenge", -1);
	gmsgWeaponInfo = REG_USER_MSG("WeaponInfo", 6);
	gmsgMapObject = REG_USER_MSG("MapObject", -1);
	gmsgMetaRender = REG_USER_MSG("MetaRender", -1);
	gmsgSpawnInit = REG_USER_MSG("SpawnInit", 3);
	gmsgPlayerVars = REG_USER_MSG("PlayerVars", -1);
	//cs16
}

LINK_ENTITY_TO_CLASS(player, CBasePlayer);

int CBasePlayer::PM_NoCollision(CBaseEntity *pEntity)
{
	if(!pEntity)
		return 0;
	int classify = pEntity->Classify();
	if(pEntity->IsPlayer() && pEntity->pev != pev)
	{
		CBasePlayer *pPlayer = (CBasePlayer *)pEntity;
		if(m_iTeam == pPlayer->m_iTeam)
			return 1;
	}
	else if(classify == CLASS_BUILDABLE)
	{
		CBaseBuildable *pBuild = (CBaseBuildable *)pEntity;
		if(!pBuild->m_pPlayer)
			return 0;

		if(m_iTeam == pBuild->m_iTeam && pBuild->m_pPlayer->pev != pev)
		{
			int iBuildClass = pBuild->GetBuildClass();

			if(iBuildClass == BUILDABLE_SENTRY || iBuildClass == BUILDABLE_DISPENSER)
				return 1;
		}
		return 0;
	}
	else if(classify == CLASS_PROJECTILE)
	{
		//player can walk through any projectiles
		return 1;
	}
	else if(pEntity->IsBSPModel() && pEntity->pev->team != 0)
	{
		if(m_iTeam == pEntity->pev->team)
			return 1;
		if(pEntity->pev->team == 3)
			return 1;
		if(g_pGameRules->m_iRoundStatus == ROUND_END)
			return 1;
	}
	return 0;
}

extern "C" int PM_NoCollision(int iPlayerIndex, int iEntIndex)
{
	edict_t *pPlayerEdict = INDEXENT(iPlayerIndex);
	edict_t *pEdict = INDEXENT(iEntIndex);
	if(!pPlayerEdict || !pEdict) return 0;

	CBasePlayer *pPlayer = (CBasePlayer *)CBaseEntity::Instance(pPlayerEdict);
	CBaseEntity *pEntity = CBaseEntity::Instance(pEdict);

	if(pPlayer->PM_NoCollision(pEntity))
		return TRUE;

	return FALSE;
}

CBasePlayer *GetRandomPlayer(int iTeam, int iClass)
{
	CBasePlayer *pPlayer = NULL;
	CBasePlayer *pTarget = NULL;
	int iBreakPoint = RANDOM_LONG(1, gpGlobals->maxClients-1);
	int iLoops = 0;
	for(int i = iBreakPoint+1; i <= gpGlobals->maxClients+1; i++)
	{
		//if hit the bound, go to the head
		if(i == gpGlobals->maxClients+1)
			i = 1;
		CBaseEntity *pEnt = UTIL_PlayerByIndex(i);
		if(pEnt && pEnt->IsPlayer())
		{
			pPlayer = (CBasePlayer *)pEnt;
			if(pPlayer->m_iTeam == iTeam && pPlayer->m_iClass == iClass)
			{
				pTarget = pPlayer;
				break;
			}
			else if(pPlayer->m_iTeam == iTeam && iClass == 0)
			{
				pTarget = pPlayer;
				break;
			}
		}
		iLoops ++;
		if(i == iBreakPoint || iLoops > gpGlobals->maxClients)
			return NULL;
	}
	return pTarget;
}

int GetClassMaxHealth(int iClass)
{
	switch(iClass)
	{
	case CLASS_SCOUT:case CLASS_SNIPER:case CLASS_SPY:case CLASS_ENGINEER:return 125;
	case CLASS_HEAVY:return 300;
	case CLASS_SOLDIER:return 200;
	case CLASS_PYRO:case CLASS_DEMOMAN:return 175;
	case CLASS_MEDIC:return 150;
	}
	return 125;
}

float GetClassMaxSpeed(int iClass)
{
	switch(iClass)
	{
	case CLASS_SCOUT: return 325;
	case CLASS_HEAVY: return 192.5;
	case CLASS_SOLDIER: return 200;
	case CLASS_PYRO: return 250;
	case CLASS_SNIPER: return 250;
	case CLASS_MEDIC: return 267.5;
	case CLASS_ENGINEER: return 250;
	case CLASS_DEMOMAN: return 232.5;
	case CLASS_SPY: return 250;
	}
	return 250;
}

int GetClassPrimary(int iClass)
{
	switch(iClass)
	{
	case CLASS_SCOUT:return WEAPON_SCATTERGUN;
	case CLASS_HEAVY:return WEAPON_MINIGUN;
	case CLASS_SOLDIER:return WEAPON_ROCKETLAUNCHER;
	case CLASS_PYRO:return WEAPON_FLAMETHROWER;
	case CLASS_SNIPER:return WEAPON_SNIPERIFLE;
	case CLASS_MEDIC:return WEAPON_SYRINGEGUN;
	case CLASS_ENGINEER:return WEAPON_SHOTGUN;
	case CLASS_DEMOMAN:return WEAPON_GRENADELAUNCHER;
	case CLASS_SPY:return WEAPON_REVOLVER;
	}
	return 0;
}

int GetClassSecondary(int iClass)
{
	switch(iClass)
	{
	case CLASS_SCOUT:case CLASS_ENGINEER:return WEAPON_PISTOL;
	case CLASS_HEAVY:case CLASS_SOLDIER:case CLASS_PYRO:return WEAPON_SHOTGUN;
	case CLASS_SNIPER:return WEAPON_SMG;
	case CLASS_MEDIC:return WEAPON_MEDIGUN;
	case CLASS_DEMOMAN:return WEAPON_STICKYLAUNCHER;
	case CLASS_SPY:return WEAPON_SAPPER;
	}
	return 0;
}

int GetClassMelee(int iClass)
{
	switch(iClass)
	{
	case CLASS_SCOUT:return WEAPON_BAT;
	case CLASS_HEAVY:return WEAPON_FIST;
	case CLASS_SOLDIER:return WEAPON_SHOVEL;
	case CLASS_PYRO:return WEAPON_FIREAXE;
	case CLASS_SNIPER:return WEAPON_KUKRI;
	case CLASS_MEDIC:return WEAPON_BONESAW;
	case CLASS_DEMOMAN:return WEAPON_BOTTLE;
	case CLASS_ENGINEER:return WEAPON_WRENCH;
	case CLASS_SPY:return WEAPON_BUTTERFLY;
	}
	return 0;
}

void CBasePlayer::SetPlayerModel()
{
	char *model;

	switch(m_iClass)
	{
		default:return;
		case CLASS_SCOUT: model = "ckf_scout"; break;
		case CLASS_HEAVY: model = "ckf_heavy"; break;
		case CLASS_SOLDIER: model = "ckf_soldier"; break;
		case CLASS_PYRO: model = "ckf_pyro"; break;
		case CLASS_SNIPER: model = "ckf_sniper"; break;
		case CLASS_MEDIC: model = "ckf_medic"; break;
		case CLASS_ENGINEER: model = "ckf_engineer"; break;
		case CLASS_DEMOMAN: model = "ckf_demoman"; break;
		case CLASS_SPY: model = "ckf_spy"; break;
	}
	
	char *color;

	if (m_iTeam == TEAM_CT)
	{
		color = COLOR_BLU;
	}
	else if (m_iTeam == TEAM_TERRORIST)
	{
		color = COLOR_RED;
	}
	else
		color = "90";

	char *infobuffer = g_engfuncs.pfnGetInfoKeyBuffer(edict());
	char *curmodel = g_engfuncs.pfnInfoKeyValue(infobuffer, "model");
	char *curcolor = g_engfuncs.pfnInfoKeyValue(infobuffer, "topcolor");
	if (strcmp(curmodel, model))
		g_engfuncs.pfnSetClientKeyValue(entindex(), infobuffer, "model", model);
	if (strcmp(curcolor, color))
	{
		g_engfuncs.pfnSetClientKeyValue(entindex(), infobuffer, "topcolor", color);
		g_engfuncs.pfnSetClientKeyValue(entindex(), infobuffer, "bottomcolor", color);
	}
	static char szModelPath[64];
	sprintf(szModelPath, "models/player/%s/%s.mdl", model, model);
	SET_MODEL(ENT(pev), szModelPath);
	m_ulModelIndexPlayer = g_sModelIndexPlayerClass[m_iClass-1];
	pev->modelindex = m_ulModelIndexPlayer;
}

void CBasePlayer::Radio(const char *msg_id, const char *msg_verbose)
{
	if (!IsPlayer())
		return;

	CBaseEntity *pEntity = NULL;

	while ((pEntity = UTIL_FindEntityByClassname(pEntity, "player")) != NULL)
	{
		if (FNullEnt(pEntity->edict()))
			break;

		if (!pEntity->IsPlayer())
			continue;

		if (pEntity->pev->flags == FL_DORMANT)
			continue;

		CBasePlayer *pOther = GetClassPtr((CBasePlayer *)pEntity->pev);

		if (pOther->pev->deadflag != DEAD_NO || pOther->m_iTeam != m_iTeam)
			continue;

		if (pOther->m_bIgnoreRadio)
			continue;

		if (msg_verbose)
			ClientPrint(pOther->pev, HUD_PRINTTALK, "#Game_radio", STRING(pev->netname), msg_verbose);

		MESSAGE_BEGIN(MSG_ONE, gmsgSendAudio, NULL, pOther->pev);
		WRITE_BYTE(ENTINDEX(edict()));
		WRITE_STRING(msg_id);
		MESSAGE_END();

		if (msg_verbose)
		{
			MESSAGE_BEGIN(MSG_ONE, SVC_TEMPENTITY, NULL, pOther->pev);
			WRITE_BYTE(TE_PLAYERATTACHMENT);
			WRITE_BYTE(ENTINDEX(edict()));
			WRITE_COORD(35);
			WRITE_SHORT(g_sModelIndexRadio);
			WRITE_SHORT(15);
			MESSAGE_END();
		}
	}
}

void CBasePlayer::Pain(int hitgroup)
{
	if (hitgroup == HITGROUP_HEAD)
	{
		if (m_iKevlar == 2)
		{
			if(m_iDisguise != DISGUISE_YES)
				EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/bhit_helmet-1.wav", VOL_NORM, ATTN_NORM);
			else
				UTIL_PlayWAV(this, "player/bhit_helmet-1.wav");
			return;
		}

		if(m_iDisguise != DISGUISE_YES)
		{
			switch (RANDOM_LONG(0, 2))
			{
				case 0: EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/headshot1.wav", VOL_NORM, ATTN_NORM); break;
				case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/headshot2.wav", VOL_NORM, ATTN_NORM); break;
				case 2: EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/headshot3.wav", VOL_NORM, ATTN_NORM); break;
			}
		}
		else
		{
			switch (RANDOM_LONG(0, 2))
			{
				case 0: UTIL_PlayWAV(this, "player/headshot1.wav");break;
				case 1: UTIL_PlayWAV(this, "player/headshot2.wav");break;
				case 2: UTIL_PlayWAV(this, "player/headshot3.wav");break;
			}
		}
	}
	if(m_iDisguise != DISGUISE_YES)
	{
		switch (RANDOM_LONG(0, 2))
		{
			case 0: EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/bhit_flesh-1.wav", VOL_NORM, ATTN_NORM); break;
			case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/bhit_flesh-2.wav", VOL_NORM, ATTN_NORM); break;
			case 2: EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/bhit_flesh-3.wav", VOL_NORM, ATTN_NORM); break;
		}
	}
	else
	{
		switch (RANDOM_LONG(0, 2))
		{
			case 0: UTIL_PlayWAV(this, "player/bhit_flesh-1.wav");break;
			case 1: UTIL_PlayWAV(this, "player/bhit_flesh-2.wav");break;
			case 2: UTIL_PlayWAV(this, "player/bhit_flesh-3.wav");break;
		}
	}
}

Vector VecVelocityForDamage(float flDamage)
{
	Vector vec(RANDOM_FLOAT(-100, 100), RANDOM_FLOAT(-100, 100), RANDOM_FLOAT(200, 300));

	if (flDamage > -50)
		vec = vec * 0.7;
	else if (flDamage > -200)
		vec = vec * 2;
	else
		vec = vec * 10;

	return vec;
}

int TrainSpeed(int iSpeed, int iMax)
{
	int iRet = 0;
	float fMax = iMax;
	float fSpeed = iSpeed;

	if (iSpeed < 0)
		iRet = TRAIN_BACK;
	else if (iSpeed == 0)
		iRet = TRAIN_NEUTRAL;
	else if (fSpeed < 0.33)
		iRet = TRAIN_SLOW;
	else if (fSpeed < 0.66)
		iRet = TRAIN_MEDIUM;
	else
		iRet = TRAIN_FAST;

	return iRet;
}

void CBasePlayer::DeathSound(void)
{
	switch (RANDOM_LONG(1, 4))
	{
		case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/die1.wav", VOL_NORM, ATTN_NORM); break;
		case 2: EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/die2.wav", VOL_NORM, ATTN_NORM); break;
		case 3: EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/die3.wav", VOL_NORM, ATTN_NORM); break;
		case 4: EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/death6.wav", VOL_NORM, ATTN_NORM); break;
	}
}

int CBasePlayer::TakeHealth(float flHealth, int bitsDamageType)
{
	return CBaseMonster::TakeHealth(flHealth, bitsDamageType);
}

Vector CBasePlayer::GetGunPosition(void)
{
	return pev->origin + pev->view_ofs;
}

void CBasePlayer::TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType, int iCrit)
{
	if(m_Cond.Invulnerable.m_iStatus)
		return;

	if(flDamage < 0)
		return;

	BOOL bShouldBleed = TRUE;
	CBaseEntity *pAttacker = (CBaseEntity *)CBaseEntity::Instance(pevAttacker);

	if (!pev->takedamage)
		return;

	if(pAttacker->IsPlayer())//no teammate damage
	{
		CBasePlayer *pPlayerAttacker = (CBasePlayer *)pAttacker;
		if(m_iTeam == pPlayerAttacker->m_iTeam && pev != pAttacker->pev)
			return;
	}

	if(m_iDisguise == DISGUISE_YES)
		bShouldBleed = FALSE;

	if(bitsDamageType & DMG_FLAME)
		bShouldBleed = FALSE;

	if(ptr)
	{
		m_LastHitGroup = ptr->iHitgroup;

		if(ptr->iHitgroup == HITGROUP_GENERIC)
			bShouldBleed = FALSE;

		switch (ptr->iHitgroup)
		{
			case HITGROUP_HEAD:
			{
				if (bShouldBleed == TRUE)
				{
					pev->punchangle.x = flDamage * -0.5;

					if (pev->punchangle.x < -12)
						pev->punchangle.x = -12;

					pev->punchangle.z = flDamage * RANDOM_FLOAT(-1, 1);

					if (pev->punchangle.z < -9)
						pev->punchangle.z = -9;
					else if (pev->punchangle.z > 9)
						pev->punchangle.z = 9;
				}

				break;
			}

			case HITGROUP_CHEST:
			{
				if (bShouldBleed == TRUE)
				{
					pev->punchangle.x = flDamage * -0.1;

					if (pev->punchangle.x < -4)
						pev->punchangle.x = -4;
				}

				break;
			}

			case HITGROUP_STOMACH:
			{
				if (bShouldBleed == TRUE)
				{
					pev->punchangle.x = flDamage * -0.1;

					if (pev->punchangle.x < -4)
						pev->punchangle.x = -4;
				}

				break;
			}

			case HITGROUP_LEFTARM:
			case HITGROUP_RIGHTARM:
			{
				bShouldBleed = FALSE;
				break;
			}

			case HITGROUP_LEFTLEG:
			case HITGROUP_RIGHTLEG:
			{
				bShouldBleed = FALSE;
				break;
			}
		}
		pev->punchangle.x *= GetPunch();
	}

	if (bShouldBleed == TRUE)
	{
		BloodSplat(ptr->vecEndPos, vecDir, ptr->iHitgroup, flDamage * 3);
		SpawnBlood(ptr->vecEndPos, BloodColor(), flDamage);
		TraceBleed(flDamage, vecDir, ptr, bitsDamageType);
	}

	m_bLastHitCrit = FALSE;

	if(iCrit >= 2) m_bLastHitCrit = TRUE;

	m_bLastHitBackStab = FALSE;

	if(bitsDamageType & DMG_BACKSTAB) m_bLastHitBackStab = TRUE;

	AddMultiDamage(pevAttacker, this, flDamage, bitsDamageType, iCrit);
}

const char *GetWeaponName(entvars_t *pevInflictor, entvars_t *pevAttacker)
{
	const char *killer_weapon_name = "world";

	if (pevAttacker->flags & FL_CLIENT)
	{
		int killer_index = ENTINDEX(ENT(pevAttacker));

		if (pevInflictor)
		{
			if (pevInflictor == pevAttacker)
			{
				CBasePlayer *pAttacker = (CBasePlayer *)CBaseEntity::Instance(pevAttacker);

				if (pAttacker)
					if (pAttacker->m_pActiveItem)
						killer_weapon_name = pAttacker->m_pActiveItem->pszName();
			}
			else
				killer_weapon_name = STRING(pevInflictor->classname);
		}
	}
	else
		killer_weapon_name = STRING(pevInflictor->classname);

	if (!strncmp(killer_weapon_name, "weapon_", 7))
		killer_weapon_name += 7;
	else if (!strncmp(killer_weapon_name, "monster_", 8))
		killer_weapon_name += 8;
	else if (strncmp(killer_weapon_name, "func_", 5))
		killer_weapon_name += 5;

	return killer_weapon_name;
}

extern char *GetTeam(int team);

void LogAttack(CBasePlayer *pAttacker, CBasePlayer *pVictim, BOOL bTeammate, int damage, int damage_armor, int health, int armor, const char *weaponname)
{
	int detail = logdetail.value;

	if (!detail)
		return;

	if (!pAttacker || !pVictim)
		return;

	if ((bTeammate && (detail & LOG_TEAMMATEATTACK)) || (!bTeammate && (detail & LOG_ENEMYATTACK)))
		UTIL_LogPrintf("\"%s<%i><%s><%s>\" attacked \"%s<%i><%s><%s>\" with \"%s\" (damage \"%d\") (damage_armor \"%d\") (health \"%d\") (armor \"%d\")\n", STRING(pAttacker->pev->netname), GETPLAYERUSERID(pAttacker->edict()), GETPLAYERAUTHID(pAttacker->edict()), GetTeam(pAttacker->m_iTeam), STRING(pVictim->pev->netname), GETPLAYERUSERID(pVictim->edict()), GETPLAYERAUTHID(pVictim->edict()), GetTeam(pVictim->m_iTeam), weaponname, damage, damage_armor, health, armor);
}

int CBasePlayer::TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType, int iCrit)
{
	if(m_Cond.Invulnerable.m_iStatus)
		return 0;

	BOOL bShowCritEffects = TRUE;
	BOOL bShowDmgFText = TRUE;

	if (bitsDamageType & (DMG_BLAST | DMG_FALL | DMG_FLAME))
		m_LastHitGroup = HITGROUP_GENERIC;

	CBaseEntity *pAttacker = CBaseEntity::Instance(pevAttacker);

	CBasePlayer *plAttacker = NULL;
	if(pAttacker->IsPlayer())
	{
		plAttacker = (CBasePlayer *)pAttacker;
		if (m_iTeam == plAttacker->m_iTeam)
		{
			bShowDmgFText = FALSE;
			if(pevAttacker == pev)
				bShowCritEffects = FALSE;
		}
	}
	else if(pAttacker->Classify() == CLASS_PROJECTILE)
	{
		if(m_iTeam == ((CGrenade *)pAttacker)->m_iTeam)
		{
			if(pevAttacker->owner == edict())
				bShowCritEffects = FALSE;
		}
	}

	//show critical effects
	if(m_iDisguise != DISGUISE_YES)
	{
		if(iCrit >= 2)
		{
			SendCriticalHit(bShowCritEffects, (pAttacker->pev != pev) ? true : false);
		}
		else if(iCrit == 1)
		{			
			SendMiniCritHit(bShowCritEffects, (pAttacker->pev != pev) ? true : false);
		}
	}

	if (bitsDamageType & DMG_BLAST)
	{
		if (!IsAlive())
			return 0;

		CBaseEntity *pInflictor = GetClassPtr((CBaseEntity *)pevInflictor);

		if (pInflictor->Classify() == CLASS_PROJECTILE)
		{
			CGrenade *pGrenade = GetClassPtr((CGrenade *)pevInflictor);

			if (pGrenade->m_iTeam == m_iTeam && pev != pevAttacker)
				return 0;
			else if(pev == pevAttacker)
				bShowDmgFText = FALSE;
		}

		if(bitsDamageType & DMG_FLAME)
		{
			AfterBurn_Add(10.0f, 3, pevAttacker, pevInflictor);
		}

		if (!FNullEnt(ENT(pevInflictor)))
			m_vecInjuredLOS = pev->origin - pevInflictor->origin;

		Pain(m_LastHitGroup);

		m_lastDamageAmount = flDamage;

		if (pev->health > flDamage && m_iDisguise != DISGUISE_YES)
		{
			SetAnimation(PLAYER_SMALL_FLINCH);
		}

		SendSpecHealth(true, pevInflictor);

		ALERT(at_console, "EXPLOSION DAMAGE %d\n", (int)flDamage);

		//add hit damage to queue
		if(bShowDmgFText && plAttacker && m_iDisguise != DISGUISE_YES && m_iCloak != CLOAK_YES)
		{
			AddHitDamage(pevAttacker, (int)flDamage, iCrit);
		}

		m_flLastDamagedTime = gpGlobals->time;
		if(plAttacker && pevAttacker != pev)
			DmgRecord_Add(plAttacker, flDamage);

		int fTookDamage = CBaseMonster::TakeDamage(pevInflictor, pevAttacker, (int)flDamage, bitsDamageType, iCrit);

		return fTookDamage;
	}

	if (!IsAlive())
		return 0;

	if (plAttacker && m_iDisguise != DISGUISE_YES)
	{
		if (plAttacker->m_iTeam == m_iTeam && pevAttacker != pev)
			return 0;

		if (plAttacker->m_pActiveItem)
		{
			if (ShouldDoLargeFlinch(m_LastHitGroup, plAttacker->m_pActiveItem->m_iId))
			{
				SetAnimation(PLAYER_LARGE_FLINCH);
			}
			else
			{
				if (m_LastHitGroup == HITGROUP_HEAD)
					m_bTakenHighDamage = (flDamage > 60);
				else
					m_bTakenHighDamage = (flDamage > 20);

				SetAnimation(PLAYER_SMALL_FLINCH);
			}
		}
	}

	if(bitsDamageType & DMG_FLAME)
	{
		AfterBurn_Add(10.0f, 3, pevAttacker, pevInflictor);
	}

	m_lastDamageAmount = flDamage;

	Pain(m_LastHitGroup);

	SendSpecHealth(false, pevInflictor);

	m_bitsHUDDamage = -1;
	m_bitsDamageType |= bitsDamageType;

	if(bShowDmgFText && plAttacker && m_iDisguise != DISGUISE_YES)
	{
		AddHitDamage(pevAttacker, (int)flDamage, iCrit);
	}

	m_flLastDamagedTime = gpGlobals->time;
	if(plAttacker && pevAttacker != pev)
		DmgRecord_Add(plAttacker, flDamage);

	return CBaseMonster::TakeDamage(pevInflictor, pevAttacker, (int)flDamage, bitsDamageType, iCrit);
}

const char *GetWeaponModelName(int iWeaponId);
int GetWeaponModelSequence(int iWeaponId);
int GetWeaponModelBody(int iWeaponId);

void CBasePlayer::PackDeadPlayerItems(void)
{
	int iWeaponRules = g_pGameRules->DeadPlayerWeapons(this);
	int iAmmoRules = g_pGameRules->DeadPlayerAmmo(this);

	if (iWeaponRules != GR_PLR_DROP_GUN_NO)
	{
		int iBestWeight = 0;
		CBasePlayerItem *pItem = NULL;

		for (int i = 0; i < MAX_ITEM_TYPES; i++)
		{
			CBasePlayerItem *pPlayerItem = m_rgpPlayerItems[i];

			while (pPlayerItem)
			{
				ItemInfo II;

				if (pPlayerItem->iItemSlot() < WEAPON_SLOT_KNIFE && pPlayerItem->GetItemInfo(&II))
				{
					if (II.iWeight > iBestWeight)
					{
						iBestWeight = II.iWeight;
						pItem = pPlayerItem;
					}
				}

				pPlayerItem = pPlayerItem->m_pNext;
			}
		}

		if (pItem && pItem->m_iId != WEAPON_FIST)//fist don't drop weaponbox
		{
			const char *modelname = GetWeaponModelName(pItem->m_iId);

			if (modelname)
			{
				CWeaponBox *pWeaponBox = (CWeaponBox *)CBaseEntity::Create("weaponbox", pev->origin, pev->angles, ENT(pev));
				pWeaponBox->pev->angles.x = 0;
				pWeaponBox->pev->angles.y = RANDOM_FLOAT(-180, 180);
				pWeaponBox->pev->velocity = pev->velocity * 0.75;
				pWeaponBox->SetThink(&CBaseEntity::SUB_Remove);
				pWeaponBox->pev->nextthink = gpGlobals->time + 60;
				pWeaponBox->m_flGivePercent = 50;
				SET_MODEL(ENT(pWeaponBox->pev), modelname);
				if(strstr(modelname, "wp_group"))
				{
					pWeaponBox->pev->sequence = GetWeaponModelSequence(pItem->m_iId);
					pWeaponBox->pev->body = GetWeaponModelBody(pItem->m_iId);
				}
			}
		}
	}

	RemoveAllItems(TRUE);
}
//no melee needed
const char *GetWeaponModelName(int iWeaponId)
{
	switch (iWeaponId)
	{
		//rf
		case WEAPON_SCATTERGUN:case WEAPON_PISTOL:
		case WEAPON_SHOTGUN:
		case WEAPON_ROCKETLAUNCHER:
		case WEAPON_FLAMETHROWER:
		case WEAPON_SNIPERIFLE:case WEAPON_SMG:
		case WEAPON_SYRINGEGUN:case WEAPON_MEDIGUN:
		case WEAPON_REVOLVER:case WEAPON_SAPPER:case WEAPON_DISGUISEKIT:
		case WEAPON_STICKYLAUNCHER:		
		case WEAPON_BUILDPDA:case WEAPON_DESTROYPDA:		
			return "models/CKF_III/wp_group_rf.mdl";
		//2bone
		case WEAPON_GRENADELAUNCHER:
		case WEAPON_MINIGUN: 
			return "models/CKF_III/wp_group_2bone.mdl";
		//for disguise
		case WEAPON_BAT:
		case WEAPON_SHOVEL:
		case WEAPON_FIREAXE:
		case WEAPON_KUKRI:
		case WEAPON_BONESAW:
		case WEAPON_WRENCH:
		case WEAPON_BOTTLE:
		case WEAPON_BUTTERFLY: return "models/CKF_III/wp_group_rf.mdl";
		case WEAPON_FIST: return "models/CKF_III/null.mdl";
		default: ALERT(at_console, "CBasePlayer::PackDeadPlayerItems(): Unhandled item- not creating weaponbox\n");break;
	}
	return NULL;
}
//no melee needed
int GetWeaponModelSequence(int iWeaponId)
{
	switch (iWeaponId)
	{
		//rf
		case WEAPON_SCATTERGUN: return 3; case WEAPON_PISTOL: return 5;
		case WEAPON_SHOTGUN: return 1;
		case WEAPON_ROCKETLAUNCHER: return 2;
		case WEAPON_FLAMETHROWER: return 23;
		case WEAPON_SNIPERIFLE: return 20; case WEAPON_SMG: return 19;
		case WEAPON_SYRINGEGUN: return 9; case WEAPON_MEDIGUN: return 10;
		case WEAPON_BUILDPDA:case WEAPON_DESTROYPDA: return 13;
		case WEAPON_STICKYLAUNCHER: return 8;
		case WEAPON_REVOLVER: return 16; case WEAPON_SAPPER: return 17; case WEAPON_DISGUISEKIT: return 15;
		//2bone
		case WEAPON_GRENADELAUNCHER: return 0;
		case WEAPON_MINIGUN: return 1;
		default: ALERT(at_console, "CBasePlayer::PackDeadPlayerItems(): Unhandled item- not creating weaponbox\n");break;
	}
	return 0;
}
//melee for disguise
int GetWeaponModelBody(int iWeaponId)
{
	switch (iWeaponId)
	{
		//rf
		case WEAPON_ROCKETLAUNCHER: return 6;
		case WEAPON_SHOTGUN: return 3;
		case WEAPON_SCATTERGUN: return 9;
		case WEAPON_PISTOL: return 15;
		case WEAPON_STICKYLAUNCHER: return 24;
		case WEAPON_SYRINGEGUN: return 27;
		case WEAPON_MEDIGUN: return 30;
		case WEAPON_BUILDPDA:case WEAPON_DESTROYPDA: return 39;
		case WEAPON_DISGUISEKIT: return 45;
		case WEAPON_REVOLVER: return 48;
		case WEAPON_SAPPER: return 51;
		case WEAPON_SMG: return 54;
		case WEAPON_SNIPERIFLE: return 57;
		case WEAPON_FLAMETHROWER: return 66;
		//for disguise
		case WEAPON_BAT: return 12;
		case WEAPON_SHOVEL: return 0;
		case WEAPON_FIREAXE: return 63;
		case WEAPON_KUKRI: return 60;
		case WEAPON_BONESAW: return 33;
		case WEAPON_WRENCH: return 36;
		case WEAPON_BOTTLE: return 18;
		case WEAPON_BUTTERFLY: return 42;
		case WEAPON_FIST: return 0;
		//2bone
		case WEAPON_GRENADELAUNCHER: return 0;
		case WEAPON_MINIGUN: return 3;
		default: ALERT(at_console, "CBasePlayer::PackDeadPlayerItems(): Unhandled item- not creating weaponbox\n");break;
	}
	return 0;
}

void CBasePlayer::GiveDefaultItems(void)
{
	RemoveAllItems(FALSE);

	//ckfgivedefaultitem
	switch(m_iClass)
	{
	default:GiveNamedItem("weapon_rocketlauncher");GiveNamedItem("weapon_smg");GiveNamedItem("weapon_bottle");break;
	case CLASS_SNIPER: GiveNamedItem("weapon_sniperifle");GiveNamedItem("weapon_smg");GiveNamedItem("weapon_kukri");break;
	case CLASS_SOLDIER: GiveNamedItem("weapon_rocketlauncher");GiveNamedItem("weapon_shotgun");GiveNamedItem("weapon_shovel");break;
	case CLASS_DEMOMAN: GiveNamedItem("weapon_grenadelauncher");GiveNamedItem("weapon_stickylauncher");GiveNamedItem("weapon_bottle");break;
	case CLASS_SCOUT: GiveNamedItem("weapon_scattergun");GiveNamedItem("weapon_pistol");GiveNamedItem("weapon_bat");break;
	case CLASS_HEAVY: GiveNamedItem("weapon_minigun");GiveNamedItem("weapon_shotgun");GiveNamedItem("weapon_fist");break;
	case CLASS_PYRO: GiveNamedItem("weapon_flamethrower");GiveNamedItem("weapon_shotgun");GiveNamedItem("weapon_fireaxe");break;
	case CLASS_SPY: GiveNamedItem("weapon_revolver");GiveNamedItem("weapon_sapper");GiveNamedItem("weapon_butterfly");GiveNamedItem("weapon_disguisekit");break;
	case CLASS_MEDIC: GiveNamedItem("weapon_syringegun");GiveNamedItem("weapon_medigun");GiveNamedItem("weapon_bonesaw");break;
	case CLASS_ENGINEER: GiveNamedItem("weapon_shotgun");GiveNamedItem("weapon_pistol");GiveNamedItem("weapon_wrench");GiveNamedItem("weapon_buildpda");GiveNamedItem("weapon_destroypda");break;
	}

	m_bHasPrimaryWeapon = TRUE;
}

void CBasePlayer::RemoveAllItems(BOOL removeSuit)
{
	int i;
	BOOL removeProgressBar = FALSE;

	if (m_pActiveItem)
	{
		ResetAutoaim();
		m_pActiveItem->Holster();
		m_pActiveItem = NULL;
	}

	m_pLastItem = NULL;

	for (i = 0; i < MAX_ITEM_TYPES; i++)
	{
		m_pActiveItem = m_rgpPlayerItems[i];

		while (m_pActiveItem)
		{
			CBasePlayerItem *pPendingItem = m_pActiveItem->m_pNext;
			m_pActiveItem->Drop();
			m_pActiveItem = pPendingItem;
		}

		m_rgpPlayerItems[i] = NULL;
	}

	m_pActiveItem = NULL;
	pev->viewmodel = 0;
	pev->weaponmodel = 0;

	if (removeSuit)
		pev->weapons = 0;
	else
		pev->weapons &= ~WEAPON_ALLWEAPONS;

	for (i = 0; i < MAX_AMMO_SLOTS; i++)
		m_rgAmmo[i] = 0;

	UpdateClientData();

	MESSAGE_BEGIN(MSG_ONE, gmsgCurWeapon, NULL, pev);
	WRITE_BYTE(0);
	WRITE_BYTE(0);
	WRITE_BYTE(0);
	MESSAGE_END();
}

void CBasePlayer::SetBombIcon(BOOL bFlash)
{
	if (m_bIsBombGuy)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, pev);

		if (bFlash)
			WRITE_BYTE(STATUSICON_FLASH);
		else
			WRITE_BYTE(STATUSICON_SHOW);

		WRITE_STRING("c4");
		WRITE_BYTE(0);
		WRITE_BYTE(160);
		WRITE_BYTE(0);
		MESSAGE_END();
	}
	else
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, pev);
		WRITE_BYTE(STATUSICON_HIDE);
		WRITE_STRING("c4");
		MESSAGE_END();
	}

	SetScoreboardAttributes();
}

void CBasePlayer::SetProgressBarTime(int iTime)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgBarTime, NULL, pev);
	WRITE_SHORT(iTime);
	MESSAGE_END();

	CBaseEntity *pEntity = NULL;
	int index = entindex();

	while ((pEntity = UTIL_FindEntityByClassname(pEntity, "player")) != NULL)
	{
		if (FNullEnt(pEntity->edict()))
			break;

		CBasePlayer *pOther = (CBasePlayer *)CBasePlayer::Instance(pEntity->pev);

		if (pOther->pev->iuser1 == OBS_IN_EYE && pOther->pev->iuser2 == index)
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgBarTime, NULL, pOther->pev);
			WRITE_SHORT(iTime);
			MESSAGE_END();
		}
	}
}

void CBasePlayer::RemoveLevelText(void)
{
}

entvars_t *g_pevLastInflictor;

void CBasePlayer::Killed(entvars_t *pevAttacker, int iGib)
{
	m_canSwitchObserverModes = false;

	if(m_LastHitGroup == HITGROUP_HEAD)
		m_bHeadShotKilled = true;
	if(m_bLastHitCrit)
		m_bCritKilled = true;
	if(m_bLastHitBackStab)
		m_bBackStabKilled = true;

	g_pGameRules->PlayerKilled(this, pevAttacker, g_pevLastInflictor);

	//清理粘弹
	ClearSticky();
	Condition_Clear();

	//解除隐身
	m_iCloak = CLOAK_NO;
	pev->renderamt = 255;
	pev->rendermode = kRenderNormal;

	//解除伪装
	Disguise_Stop();

	//开始重生
	Respawn_Start();

	//Clear carrying building
	if(m_pCarryBuild)
	{
		m_pCarryBuild->Killed(pevAttacker, 0);
		m_pCarryBuild = NULL;
	}

	//Clear carrying blueprint
	m_iCarryBluePrint = 0;

	//Clear healer
	m_pHealer = NULL;
	m_iHealerCount = 0;

	if (m_pTank)
	{
		m_pTank->Use(this, this, USE_OFF, 0);
		m_pTank = NULL;
	}

	CSound *pSound = CSoundEnt::SoundPointerForIndex(CSoundEnt::ClientSoundIndex(edict()));

	if (pSound)
		pSound->Reset();

	SetAnimation(PLAYER_DIE);

	pev->modelindex = m_ulModelIndexPlayer;
	pev->deadflag = DEAD_DYING;
	pev->movetype = MOVETYPE_TOSS;
	pev->takedamage = DAMAGE_NO;
	pev->flags &= ~FL_ONGROUND;

	pev->gamestate = 1;

	if (!fadetoblack.value)
	{
		pev->iuser1 = OBS_CHASE_FREE;
		pev->iuser2 = ENTINDEX(ENT(pev));
		pev->iuser3 = ENTINDEX(ENT(pevAttacker));
		m_hObserverTarget = UTIL_PlayerByIndex(pev->iuser3);
	}
	else
		UTIL_ScreenFade(this, Vector(0, 0, 0), 3, 3, 255, FFADE_OUT | FFADE_STAYOUT);

	SetScoreboardAttributes();

	if (m_iThrowDirection)
	{
		switch (m_iThrowDirection)
		{
			case 1:
			{
				UTIL_MakeVectors(pev->angles);
				pev->velocity = gpGlobals->v_forward * RANDOM_FLOAT(100, 200);
				pev->velocity.z = RANDOM_FLOAT(50, 100);
				break;
			}

			case 2:
			{
				UTIL_MakeVectors(pev->angles);
				pev->velocity = gpGlobals->v_forward * RANDOM_FLOAT(-100, -200);
				pev->velocity.z = RANDOM_FLOAT(50, 100);
				break;
			}

			case 3:
			{
				if (FClassnameIs(pevAttacker, "player"))
				{
					UTIL_MakeVectors(pevAttacker->angles);
					pev->velocity = gpGlobals->v_forward * RANDOM_FLOAT(200, 300);
					pev->velocity.z = RANDOM_FLOAT(200, 300);
				}

				break;
			}

			case 4:
			{
				pev->velocity = m_vecInjuredLOS * (1 / m_vecInjuredLOS.Length()) * (2300 - m_vecInjuredLOS.Length()) * 0.25;
				pev->velocity.z = (2300 - m_vecInjuredLOS.Length()) / 2.75;
				break;
			}

			case 5:
			{
				pev->velocity = m_vecInjuredLOS * (1 / m_vecInjuredLOS.Length()) * (500 - m_vecInjuredLOS.Length());
				pev->velocity.z = (350 - m_vecInjuredLOS.Length()) * 1.5;
				break;
			}

			case 6:
			{
				if (FClassnameIs(pevAttacker, "player"))
				{
					UTIL_MakeVectors(pevAttacker->angles);
					pev->velocity = gpGlobals->v_forward * RANDOM_FLOAT(200, 300);
				}

				break;
			}
		}

		m_iThrowDirection = 0;
	}

	SetSuitUpdate(NULL, FALSE, 0);

	m_iClientHealth = 0;
	MESSAGE_BEGIN(MSG_ONE, gmsgHealth, NULL, pev);
	WRITE_SHORT(m_iClientHealth);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ONE, gmsgCurWeapon, NULL, pev);
	WRITE_BYTE(0);
	WRITE_BYTE(0xFF);
	WRITE_BYTE(0xFF);
	MESSAGE_END();

	m_iFOV = m_iClientFOV = 0;

	MESSAGE_BEGIN(MSG_ONE, gmsgSetFOV, NULL, pev);
	WRITE_BYTE(0);
	MESSAGE_END();

	g_pGameRules->CheckWinConditions();
	m_bNotKilled = false;

	SetThink(&CBasePlayer::PlayerDeathThink);
	pev->nextthink = gpGlobals->time + 0.1;
	pev->solid = SOLID_NOT;

	if ((pev->health < -9000 && iGib != GIB_NEVER) || iGib == GIB_ALWAYS)
	{
		pev->solid = SOLID_NOT;
		GibMonster();
		pev->effects |= EF_NODRAW;
		g_pGameRules->CheckWinConditions();
		return;
	}

	DeathSound();
	pev->angles.x = 0;
	pev->angles.z = 0;
	
	/*if (!(m_flDisplayHistory & DHF_SPEC_DUCK))
	{
		HintMessage("#Spec_Duck", TRUE, TRUE);
		m_flDisplayHistory |= DHF_SPEC_DUCK;
	}*/
}

int CBasePlayer::IsBombGuy(void)
{
	if (!g_pGameRules->IsMultiplayer())
		return 0;

	return m_bIsBombGuy;
}

void CBasePlayer::SetDisguiseAnimation(PLAYER_ANIM playerAnim)
{
	if(m_iDisguise != DISGUISE_YES)
		return;

	if (!pev->modelindex)
		return;

	if (!(playerAnim == PLAYER_SMALL_FLINCH || playerAnim == PLAYER_LARGE_FLINCH || gpGlobals->time > m_flFlinchTime || pev->health <= 0))
		return;

	float speed = pev->velocity.Length2D();

	if (FBitSet(pev->flags, FL_FROZEN))
	{
		speed = 0;
		playerAnim = PLAYER_IDLE;
	}

	int animDesired;
	int animStandDesired;
	int animHop = LookupActivity(ACT_HOP);
	int animLeap = LookupActivity(ACT_LEAP);

	char szAnim[64];

	switch (m_IdealActivity)
	{
		case ACT_HOP:
		case ACT_LEAP:
		{
			if (m_Activity == m_IdealActivity)
				return;

			if (m_Activity == ACT_RANGE_ATTACK1)
				strcpy(szAnim, "ref_shoot_");
			else if (m_Activity == ACT_RANGE_ATTACK2)
				strcpy(szAnim, "ref_shoot2_");
			else if (m_Activity == ACT_RELOAD)
				strcpy(szAnim, "ref_reload_");
			else
				strcpy(szAnim, "ref_aim_");

			strcat(szAnim, m_szDisguiseAnimExtention);
			animDesired = LookupSequence(szAnim);

			if (animDesired == -1)
				animDesired = 0;

			animStandDesired = animDesired;

			if (m_IdealActivity == ACT_LEAP)
			{
				m_Activity = m_IdealActivity;
			}
			else
			{
				m_Activity = m_IdealActivity;
			}

			break;
		}

		case ACT_RELOAD:
		{
			if (FBitSet(pev->flags, FL_DUCKING))
				strcpy(szAnim, "crouch_reload_");
			else
				strcpy(szAnim, "ref_reload_");

			strcat(szAnim, m_szDisguiseAnimExtention);
			animDesired = LookupSequence(szAnim);

			if (animDesired == -1)
				animDesired = 0;

			animStandDesired = animDesired;

			if(FBitSet(pev->flags, FL_DUCKING))
			{
				strcpy(szAnim, "ref_reload_");
				strcat(szAnim, m_szDisguiseAnimExtention);
				animStandDesired = LookupSequence(szAnim);

				if (animStandDesired == -1)
					animStandDesired = 0;
			}

			if (m_iDisguiseSequence != animDesired || !m_fSequenceLoops)
				pev->frame = 0;

			if (!m_fSequenceLoops)
				pev->effects |= EF_NOINTERP;

			m_Activity = m_IdealActivity;
			break;
		}

		case ACT_HOLDBOMB:
		{
			if (FBitSet(pev->flags, FL_DUCKING))
				strcpy(szAnim, "crouch_aim_");
			else
				strcpy(szAnim, "ref_aim_");

			strcat(szAnim, m_szDisguiseAnimExtention);
			animDesired = LookupSequence(szAnim);

			if (animDesired == -1)
				animDesired = 0;

			animStandDesired = animDesired;

			if(FBitSet(pev->flags, FL_DUCKING))
			{
				strcpy(szAnim, "ref_aim_");
				strcat(szAnim, m_szDisguiseAnimExtention);
				animStandDesired = LookupSequence(szAnim);

				if (animStandDesired == -1)
					animStandDesired = 0;
			}

			m_Activity = m_IdealActivity;
			break;
		}

		case ACT_WALK:
		{
			if ((m_Activity != ACT_RANGE_ATTACK1 || m_fSequenceFinished) && (m_Activity != ACT_RANGE_ATTACK2 || m_fSequenceFinished) && (m_Activity != ACT_FLINCH_SMALL || m_fSequenceFinished) && (m_Activity != ACT_FLINCH_LARGE || m_fSequenceFinished) && (m_Activity != ACT_RELOAD || m_fSequenceFinished))
			{
				if (speed <= 135 || gpGlobals->time <= m_flLastAttackTime + 4)
				{
					if (FBitSet(pev->flags, FL_DUCKING))
						strcpy(szAnim, "crouch_aim_");
					else
						strcpy(szAnim, "ref_aim_");

					strcat(szAnim, m_szDisguiseAnimExtention);
					animDesired = LookupSequence(szAnim);

					if (animDesired == -1)
						animDesired = 0;

					animStandDesired = animDesired;

					if(FBitSet(pev->flags, FL_DUCKING))
					{
						strcpy(szAnim, "ref_aim_");
						strcat(szAnim, m_szDisguiseAnimExtention);
						animStandDesired = LookupSequence(szAnim);

						if (animStandDesired == -1)
							animStandDesired = 0;
					}

					m_Activity = ACT_WALK;
				}
				else
				{
					strcpy(szAnim, "run_");
					strcat(szAnim, m_szDisguiseAnimExtention);
					animDesired = LookupSequence(szAnim);

					if (animDesired == -1)
					{
						if (FBitSet(pev->flags, FL_DUCKING))
							strcpy(szAnim, "crouch_aim_");
						else
							strcpy(szAnim, "ref_aim_");

						strcat(szAnim, m_szDisguiseAnimExtention);
						animDesired = LookupSequence(szAnim);

						if (animDesired == -1)
							animDesired = 0;

						animStandDesired = animDesired;

						if(FBitSet(pev->flags, FL_DUCKING))
						{
							strcpy(szAnim, "ref_aim_");
							strcat(szAnim, m_szDisguiseAnimExtention);
							animStandDesired = LookupSequence(szAnim);

							if (animStandDesired == -1)
								animStandDesired = 0;
						}

						m_Activity = ACT_RUN;
					}
					else
					{
						m_Activity = ACT_RUN;
					}
				}
			}

			break;
		}
		case ACT_FLINCH_SMALL:
		case ACT_FLINCH_LARGE:
		{
			break;
		}
	}

	if (pev->gaitsequence != animHop && pev->gaitsequence != animLeap)
	{
		if (speed > 135)
		{
			if (gpGlobals->time > m_flLastAttackTime + 4)
			{
				//if (m_Activity != ACT_FLINCH_SMALL && m_Activity != ACT_FLINCH_LARGE)
				//{
					strcpy(szAnim, "run_");
					strcat(szAnim, m_szDisguiseAnimExtention);
					animDesired = LookupSequence(szAnim);

					if (animDesired == -1)
					{
						if (FBitSet(pev->flags, FL_DUCKING))
							strcpy(szAnim, "crouch_aim_");
						else
							strcpy(szAnim, "ref_aim_");

						strcat(szAnim, m_szDisguiseAnimExtention);
						animDesired = LookupSequence(szAnim);
					}

					if (animDesired == -1)
						animDesired = 0;

					animStandDesired = animDesired;

					if(strstr(szAnim, "crouch"))
					{
						strcpy(szAnim, "ref_aim_");
						strcat(szAnim, m_szDisguiseAnimExtention);
						animStandDesired = LookupSequence(szAnim);

						if (animStandDesired == -1)
							animStandDesired = 0;
					}

					m_Activity = ACT_RUN;
				//}
			}
		}
	}

	if (m_iDisguiseSequence == animDesired)
		return;

	m_iDisguiseSequence = animDesired;

	m_iDisguiseStandSequence = animStandDesired;
}

void CBasePlayer::SetAnimation(PLAYER_ANIM playerAnim)
{
	if (!pev->modelindex)
		return;

	if (!(playerAnim == PLAYER_SMALL_FLINCH || playerAnim == PLAYER_LARGE_FLINCH || gpGlobals->time > m_flFlinchTime || pev->health <= 0))
		return;

	//if(m_iFOV < m_iDefaultFOV && playerAnim == PLAYER_IDLE)
	//	return;

	float speed = pev->velocity.Length2D();

	if (FBitSet(pev->flags, FL_FROZEN))
	{
		speed = 0;
		playerAnim = PLAYER_IDLE;
	}

	int animDesired;
	int animHop = LookupActivity(ACT_HOP);
	int animLeap = LookupActivity(ACT_LEAP);

	switch (playerAnim)
	{
		case PLAYER_JUMP:
		{
			if (m_Activity == ACT_SWIM || m_Activity == ACT_DIESIMPLE || m_Activity == ACT_HOVER)
				m_IdealActivity = m_Activity;
			else
				m_IdealActivity = ACT_HOP;

			break;
		}

		case PLAYER_SUPERJUMP:
		{
			if (m_Activity == ACT_SWIM || m_Activity == ACT_DIESIMPLE || m_Activity == ACT_HOVER)
				m_IdealActivity = m_Activity;
			else
				m_IdealActivity = ACT_LEAP;

			break;
		}

		case PLAYER_DIE:
		{
			m_IdealActivity = ACT_DIESIMPLE;
			DeathSound();
			break;
		}

		case PLAYER_ATTACK1:
		{
			if (m_Activity == ACT_SWIM || m_Activity == ACT_DIESIMPLE || m_Activity == ACT_HOVER)
				m_IdealActivity = m_Activity;
			else
				m_IdealActivity = ACT_RANGE_ATTACK1;

			break;
		}

		case PLAYER_ATTACK1_RIGHT:
		{
			if (m_Activity == ACT_SWIM || m_Activity == ACT_DIESIMPLE || m_Activity == ACT_HOVER)
				m_IdealActivity = m_Activity;
			else
				m_IdealActivity = ACT_RANGE_ATTACK2;

			break;
		}

		case PLAYER_RELOAD:
		{
			if (m_Activity == ACT_SWIM || m_Activity == ACT_DIESIMPLE || m_Activity == ACT_HOVER)
				m_IdealActivity = m_Activity;
			else
				m_IdealActivity = ACT_RELOAD;

			break;
		}

		case PLAYER_IDLE:
		case PLAYER_WALK:
		{
			if (FBitSet(pev->flags, FL_ONGROUND) || (m_Activity != ACT_HOP && m_Activity != ACT_LEAP))
			{
				if (pev->waterlevel <= 1)
					m_IdealActivity = ACT_WALK;
				else if (!speed)
					m_IdealActivity = ACT_HOVER;
				else
					m_IdealActivity = ACT_SWIM;
			}
			else
				m_IdealActivity = m_Activity;

			break;
		}

		case PLAYER_HOLDBOMB: m_IdealActivity = ACT_HOLDBOMB; break;
		case PLAYER_SMALL_FLINCH: m_IdealActivity = ACT_FLINCH_SMALL; break;
		case PLAYER_LARGE_FLINCH: m_IdealActivity = ACT_FLINCH_LARGE; break;
	}

	SetDisguiseAnimation(playerAnim);

	char szAnim[64];
	BOOL fReloadSequence = FALSE;

	if(m_pActiveItem)
	{
		if(m_pActiveItem->m_iId == WEAPON_MINIGUN)
		{
#define STATE_MINIGUN_NONE 0
#define STATE_MINIGUN_WINDUP 1
#define STATE_MINIGUN_SPIN 2
#define STATE_MINIGUN_FIRE 3
#define STATE_MINIGUN_FIRECRIT 4
#define STATE_MINIGUN_WINDDOWN 5

			CMinigun *minigun = (CMinigun *)m_pActiveItem;
			if( (minigun->m_iSpin >= STATE_MINIGUN_WINDUP && minigun->m_iSpin <= STATE_MINIGUN_SPIN) || minigun->m_iSpin == STATE_MINIGUN_WINDDOWN)
			{
				fReloadSequence = TRUE;
			}			
		}
		if(m_pActiveItem->m_iId == WEAPON_SNIPERIFLE)
		{
			CSniperifle *sniperifle = (CSniperifle *)m_pActiveItem;
			if( sniperifle->m_iWeaponState & WEAPONSTATE_CHARGING )
			{
				fReloadSequence = TRUE;
			}			
		}
	}

	switch (m_IdealActivity)
	{
		case ACT_HOP:
		case ACT_LEAP:
		{
			if (m_Activity == m_IdealActivity)
				return;

			if (m_Activity == ACT_RANGE_ATTACK1)
				strcpy(szAnim, "ref_shoot_");
			else if (m_Activity == ACT_RANGE_ATTACK2)
				strcpy(szAnim, "ref_shoot2_");
			else if (m_Activity == ACT_RELOAD || fReloadSequence)
				strcpy(szAnim, "ref_reload_");
			else
				strcpy(szAnim, "ref_aim_");

			strcat(szAnim, m_szAnimExtention);
			animDesired = LookupSequence(szAnim);

			if (animDesired == -1)
				animDesired = 0;

			if (pev->sequence != animDesired || !m_fSequenceLoops)
				pev->frame = 0;

			if (!m_fSequenceLoops)
				pev->effects |= EF_NOINTERP;

			if (m_IdealActivity == ACT_LEAP)
			{
				pev->gaitsequence = LookupActivity(ACT_LEAP);
				m_Activity = m_IdealActivity;
			}
			else
			{
				pev->gaitsequence = LookupActivity(ACT_HOP);
				m_Activity = m_IdealActivity;
			}

			break;
		}

		case ACT_RANGE_ATTACK1:
		{
			m_flLastAttackTime = gpGlobals->time;

			if (FBitSet(pev->flags, FL_DUCKING))
				strcpy(szAnim, "crouch_shoot_");
			else
				strcpy(szAnim, "ref_shoot_");

			strcat(szAnim, m_szAnimExtention);
			animDesired = LookupSequence(szAnim);

			if (animDesired == -1)
				animDesired = 0;

			pev->sequence = animDesired;
			pev->frame = 0;
			ResetSequenceInfo();
			m_Activity = m_IdealActivity;
			break;
		}

		case ACT_RANGE_ATTACK2:
		{
			m_flLastAttackTime = gpGlobals->time;

			if (FBitSet(pev->flags, FL_DUCKING))
				strcpy(szAnim, "crouch_shoot2_");
			else
				strcpy(szAnim, "ref_shoot2_");

			strcat(szAnim, m_szAnimExtention);
			animDesired = LookupSequence(szAnim);

			if (animDesired == -1)
				animDesired = 0;

			pev->sequence = animDesired;
			pev->frame = 0;
			ResetSequenceInfo();
			m_Activity = m_IdealActivity;
			break;
		}

		case ACT_RELOAD:
		{
			if(m_iDisguise == DISGUISE_YES)
				return;

			if (FBitSet(pev->flags, FL_DUCKING))
				strcpy(szAnim, "crouch_reload_");
			else
				strcpy(szAnim, "ref_reload_");

			strcat(szAnim, m_szAnimExtention);
			animDesired = LookupSequence(szAnim);

			if (animDesired == -1)
				animDesired = 0;

			if (pev->sequence != animDesired || !m_fSequenceLoops)
				pev->frame = 0;

			if (!m_fSequenceLoops)
				pev->effects |= EF_NOINTERP;

			m_Activity = m_IdealActivity;
			break;
		}

		case ACT_HOLDBOMB:
		{
			if (FBitSet(pev->flags, FL_DUCKING))
				strcpy(szAnim, "crouch_aim_");
			else
				strcpy(szAnim, "ref_aim_");

			strcat(szAnim, m_szAnimExtention);
			animDesired = LookupSequence(szAnim);

			if (animDesired == -1)
				animDesired = 0;

			m_Activity = m_IdealActivity;
			break;
		}

		case ACT_WALK:
		{
			if ((m_Activity != ACT_RANGE_ATTACK1 || m_fSequenceFinished) && (m_Activity != ACT_RANGE_ATTACK2 || m_fSequenceFinished) && (m_Activity != ACT_FLINCH_SMALL || m_fSequenceFinished) && (m_Activity != ACT_FLINCH_LARGE || m_fSequenceFinished) && (m_Activity != ACT_RELOAD || m_fSequenceFinished))
			{
				if (speed <= 135 || gpGlobals->time <= m_flLastAttackTime + 4)
				{
					if (FBitSet(pev->flags, FL_DUCKING))
						strcpy(szAnim, "crouch_");
					else
						strcpy(szAnim, "ref_");

					if(fReloadSequence)
						strcat(szAnim, "reload_");
					else
						strcat(szAnim, "aim_");

					strcat(szAnim, m_szAnimExtention);
					animDesired = LookupSequence(szAnim);

					if (animDesired == -1)
						animDesired = 0;

					m_Activity = ACT_WALK;
				}
				else
				{
					strcpy(szAnim, "run_");
					strcat(szAnim, m_szAnimExtention);
					animDesired = LookupSequence(szAnim);

					if (animDesired == -1)
					{
						if (FBitSet(pev->flags, FL_DUCKING))
							strcpy(szAnim, "crouch_");
						else
							strcpy(szAnim, "ref_");

						if(fReloadSequence)
							strcat(szAnim, "reload_");
						else
							strcat(szAnim, "aim_");

						if(m_iDisguise == DISGUISE_YES)
							strcat(szAnim, m_szDisguiseAnimExtention);
						else
							strcat(szAnim, m_szAnimExtention);
						animDesired = LookupSequence(szAnim);

						if (animDesired == -1)
							animDesired = 0;

						m_Activity = ACT_RUN;
						pev->gaitsequence = LookupActivity(ACT_RUN);
					}
					else
					{
						m_Activity = ACT_RUN;
						pev->gaitsequence = animDesired;
					}
				}
			}
			else
			{
				animDesired = pev->sequence;
			}
			if (speed <= 135)
				pev->gaitsequence = LookupActivity(ACT_WALK);
			else
				pev->gaitsequence = LookupActivity(ACT_RUN);

			break;
		}

		case ACT_FLINCH_SMALL:
		case ACT_FLINCH_LARGE:
		{
			if(m_iDisguise == DISGUISE_YES)
				return;
			m_Activity = m_IdealActivity;

			switch (m_LastHitGroup)
			{
				case HITGROUP_GENERIC:
				{
					if (RANDOM_LONG(0, 1))
						animDesired = LookupSequence("gut_flinch");
					else
						animDesired = LookupSequence("head_flinch");

					break;
				}

				case HITGROUP_HEAD:
				case HITGROUP_CHEST:
				{
					animDesired = LookupSequence("head_flinch");
					break;
				}

				default: animDesired = LookupSequence("gut_flinch");
			}

			if (animDesired == -1)
				animDesired = 0;

			break;
		}

		case ACT_DIESIMPLE:
		{
			if (m_Activity != m_IdealActivity)
			{
				m_Activity = m_IdealActivity;
				m_flDeathThrowTime = 0;
				m_iThrowDirection = 0;

				switch (m_LastHitGroup)
				{
					case HITGROUP_GENERIC:
					{
						switch (RANDOM_LONG(0, 8))
						{
							case 0: animDesired = LookupActivity(ACT_DIE_HEADSHOT); m_iThrowDirection = 2; break;
							case 1: animDesired = LookupActivity(ACT_DIE_GUTSHOT); break;
							case 2: animDesired = LookupActivity(ACT_DIE_BACKSHOT); m_iThrowDirection = 3; break;
							case 3: animDesired = LookupActivity(ACT_DIESIMPLE); break;
							case 4: animDesired = LookupActivity(ACT_DIEBACKWARD); m_iThrowDirection = 3; break;
							case 5: animDesired = LookupActivity(ACT_DIEFORWARD); m_iThrowDirection = 1; break;
							case 6: animDesired = LookupActivity(ACT_DIE_CHESTSHOT); break;
							case 7: animDesired = LookupActivity(ACT_DIE_GUTSHOT); break;
							case 8: animDesired = LookupActivity(ACT_DIE_HEADSHOT); break;
						}

						break;
					}

					case HITGROUP_HEAD:
					{
						int random = RANDOM_LONG(0, 8);
						m_bHeadShotKilled = TRUE;

						if (m_bTakenHighDamage == TRUE)
							random++;

						switch (random)
						{
							case 0: m_iThrowDirection = 0; break;
							case 1: m_iThrowDirection = 2; break;
							case 2: m_iThrowDirection = 2; break;
							case 3: m_iThrowDirection = 1; break;
							case 4: m_iThrowDirection = 1; break;
							case 5: m_iThrowDirection = 3; break;
							case 6: m_iThrowDirection = 3; break;
							case 7: m_iThrowDirection = 0; break;
							case 8: m_iThrowDirection = 0; break;
						}

						animDesired = LookupActivity(ACT_DIE_HEADSHOT);
						break;
					}

					case HITGROUP_CHEST: animDesired = LookupActivity(ACT_DIE_CHESTSHOT); break;
					case HITGROUP_STOMACH: animDesired = LookupActivity(ACT_DIE_GUTSHOT); break;
					case HITGROUP_LEFTARM: animDesired = LookupSequence("left"); break;
					case HITGROUP_RIGHTARM:
					{
						m_iThrowDirection = RANDOM_LONG(0, 1) ? 3 : 6;
						animDesired = LookupSequence("right");
						break;
					}

					case HITGROUP_LEFTLEG:
					case HITGROUP_RIGHTLEG: animDesired = LookupActivity(ACT_DIESIMPLE); break;
				}

				if (FBitSet(pev->flags, FL_DUCKING))
				{
					animDesired = LookupSequence("crouch_die");
					m_iThrowDirection = 2;
				}
				else
				{
					if (m_bInjuredBlast == TRUE || m_bInjuredExplosion == TRUE)
					{
						UTIL_MakeVectors(pev->angles);

						if (DotProduct(gpGlobals->v_forward, m_vecInjuredLOS) > 0)
							animDesired = LookupSequence("left");
						else if (RANDOM_LONG(0, 1))
							animDesired = LookupSequence("crouch_die");
						else
							animDesired = LookupActivity(ACT_DIE_HEADSHOT);

						if (m_bInjuredBlast == TRUE)
							m_iThrowDirection = 4;
						else if (m_bInjuredExplosion == TRUE)
							m_iThrowDirection = 5;
					}
				}

				if (animDesired == -1)
					animDesired = 0;

				if (pev->sequence != animDesired)
				{
					pev->gaitsequence = 0;
					pev->sequence = animDesired;
					pev->frame = 0;
					ResetSequenceInfo();
				}
			}

			return;
		}

		default:
		{
			if (m_Activity == m_IdealActivity)
				return;

			m_Activity = m_IdealActivity;
			animDesired = LookupActivity(m_IdealActivity);

			if (pev->gaitsequence != animDesired)
			{
				pev->gaitsequence = 0;
				pev->sequence = animDesired;
				pev->frame = 0;
				ResetSequenceInfo();
			}

			return;
		}
	}

	if (pev->gaitsequence != animHop && pev->gaitsequence != animLeap)
	{
		if (FBitSet(pev->flags, FL_DUCKING))
		{
			if (speed)
				pev->gaitsequence = LookupActivity(ACT_CROUCH);
			else
				pev->gaitsequence = LookupActivity(ACT_CROUCHIDLE);
		}
		else if (speed > 135)
		{
			if (gpGlobals->time > m_flLastAttackTime + 4)
			{
				if (m_Activity != ACT_FLINCH_SMALL && m_Activity != ACT_FLINCH_LARGE)
				{
					strcpy(szAnim, "run_");

					if(m_iDisguise == DISGUISE_YES)
						strcat(szAnim, m_szDisguiseAnimExtention);
					else
						strcat(szAnim, m_szAnimExtention);

					animDesired = LookupSequence(szAnim);

					if (animDesired == -1)
					{
						if (FBitSet(pev->flags, FL_DUCKING))
							strcpy(szAnim, "crouch_");
						else
							strcpy(szAnim, "ref_");

						if(fReloadSequence)
							strcat(szAnim, "reload_");
						else
							strcat(szAnim, "aim_");

						if(m_iDisguise == DISGUISE_YES)
							strcat(szAnim, m_szDisguiseAnimExtention);
						else
							strcat(szAnim, m_szAnimExtention);
						animDesired = LookupSequence(szAnim);
					}
					else
						pev->gaitsequence = animDesired;

					m_Activity = ACT_RUN;
				}
			}

			pev->gaitsequence = LookupActivity(ACT_RUN);
		}
		else if (speed > 0)
			pev->gaitsequence = LookupActivity(ACT_WALK);
		else
			pev->gaitsequence = LookupActivity(ACT_IDLE);
	}

	if (pev->sequence == animDesired)
		return;

	pev->sequence = animDesired;
	pev->frame = 0;
	ResetSequenceInfo();
}

#define AIRTIME 12

void CBasePlayer::WaterMove(void)
{
	if (pev->movetype == MOVETYPE_NOCLIP || pev->movetype == MOVETYPE_NONE)
		return;

	if (pev->health < 0)
		return;

	if (pev->waterlevel != 3)
	{
		if (pev->air_finished < gpGlobals->time)
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/pl_wade1.wav", VOL_NORM, ATTN_NORM);
		else if (pev->air_finished < gpGlobals->time + 9)
			EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/pl_wade2.wav", VOL_NORM, ATTN_NORM);

		pev->air_finished = gpGlobals->time + 12;
		pev->dmg = 2;

		if (m_idrowndmg > m_idrownrestored)
		{
			m_rgbTimeBasedDamage[itbd_DrownRecover] = 0;
			m_bitsDamageType &= ~DMG_DROWN;
			m_bitsDamageType |= DMG_DROWNRECOVER;
		}
	}
	else
	{
		m_rgbTimeBasedDamage[itbd_DrownRecover] = 0;
		m_bitsDamageType &= ~DMG_DROWNRECOVER;

		if (gpGlobals->time > pev->air_finished)
		{
			if (gpGlobals->time > pev->pain_finished)
			{
				pev->dmg += 1;

				if (pev->dmg > 5)
					pev->dmg = 5;

				TakeDamage(VARS(eoNullEntity), VARS(eoNullEntity), pev->dmg, DMG_DROWN, 0);
				pev->pain_finished = gpGlobals->time + 1;
				m_idrowndmg += pev->dmg;
			}
		}
		else
			m_bitsDamageType &= ~DMG_DROWN;
	}

	if (!pev->waterlevel)
	{
		if (pev->flags & FL_INWATER)
			pev->flags &= ~FL_INWATER;

		return;
	}

	int air = (int)(pev->air_finished - gpGlobals->time);

	if (!RANDOM_LONG(0, 31) && RANDOM_LONG(0, AIRTIME - 1) >= air)
	{
		switch (RANDOM_LONG(0, 3))
		{
			case 0: EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_swim1.wav", 0.8, ATTN_NORM); break;
			case 1: EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_swim2.wav", 0.8, ATTN_NORM); break;
			case 2: EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_swim3.wav", 0.8, ATTN_NORM); break;
			case 3: EMIT_SOUND(ENT(pev), CHAN_BODY, "player/pl_swim4.wav", 0.8, ATTN_NORM); break;
		}
	}

	if (pev->watertype == CONTENT_LAVA)
	{
		if (pev->dmgtime < gpGlobals->time)
			TakeDamage(VARS(eoNullEntity), VARS(eoNullEntity), pev->waterlevel * 10, DMG_BURN, 0);
	}
	else if (pev->watertype == CONTENT_SLIME)
	{
		pev->dmgtime = gpGlobals->time + 1;
		TakeDamage(VARS(eoNullEntity), VARS(eoNullEntity), pev->waterlevel * 4, DMG_ACID, 0);
	}

	if (!(pev->flags & FL_INWATER))
	{
		pev->flags |= FL_INWATER;
		pev->dmgtime = 0;
	}
}

BOOL CBasePlayer::IsOnLadder(void)
{
	return pev->movetype == MOVETYPE_FLY;
}

void CBasePlayer::AddAccount(int iAmount, BOOL flash)
{
	m_iAccount += iAmount;

	if (m_iAccount < 0)
		m_iAccount = 0;
	else if (m_iAccount > 16000)
		m_iAccount = 16000;

	MESSAGE_BEGIN(MSG_ONE, gmsgMoney, NULL, pev);
	WRITE_LONG(m_iAccount);
	WRITE_BYTE(flash);
	MESSAGE_END();
}

void CBasePlayer::SyncRoundTimer(void)
{
	g_pGameRules->SyncRoundTimer();
}

void CBasePlayer::ResetMenu(void)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgShowMenu, NULL, pev);
	WRITE_SHORT(0);
	WRITE_CHAR(0);
	WRITE_BYTE(0);
	WRITE_STRING("");
	MESSAGE_END();
}

extern void CheckStartMoney(void);

void CBasePlayer::JoiningThink(void)
{
	switch (m_iJoiningState)
	{
		case JOINED: return;
		case SHOWLTEXT:
		{
			RemoveLevelText();
			m_iJoiningState = SHOWTEAMSELECT;

			m_fLastMovement = gpGlobals->time;
			break;
		}

		case READINGLTEXT:
		{
			if (m_afButtonPressed & (IN_ATTACK | IN_ATTACK2 | IN_JUMP))
			{
				m_afButtonPressed &= ~(IN_ATTACK | IN_ATTACK2 | IN_JUMP);
				RemoveLevelText();
				m_iJoiningState = SHOWTEAMSELECT;
			}

			break;
		}

		case GETINTOGAME:
		{
			m_bNotKilled = FALSE;
			m_iIgnoreMessage = IGNOREMSG_NONE;
			m_iTeamKills = 0;
			m_iFOV = m_iDefaultFOV;//Reset Fov
			m_bJustConnected = FALSE;
			ResetMaxSpeed();
			m_iJoiningState = JOINED;

			if (g_pGameRules->FPlayerCanRespawn(this))
			{
				Spawn();
			}
			else
			{
				pev->deadflag = DEAD_RESPAWNABLE;

				if (pev->classname)
					RemoveEntityHashValue(pev, STRING(pev->classname), CLASSNAME);

				pev->classname = MAKE_STRING("player");
				AddEntityHashValue(pev, STRING(pev->classname), CLASSNAME);

				pev->flags &= FL_PROXY;
				pev->flags |= (FL_SPECTATOR | FL_CLIENT);
				
				edict_t *pSpot = (edict_t *)g_pGameRules->GetPlayerSpawnSpot(this);
				entvars_t *pevSpot = VARS(pSpot);
				StartObserver(pevSpot->origin, pevSpot->angles);
				Observer_SetMode(OBS_ROAMING);

				//shall we really check this? hack hack
				g_pGameRules->CheckWinConditions();

				MESSAGE_BEGIN(MSG_ALL, gmsgTeamInfo);
				WRITE_BYTE(ENTINDEX(edict()));
				WRITE_STRING(GetTeam(m_iTeam));
				MESSAGE_END();

				pev->frags = 0;
				m_iDominates = 0;

				SendScoreInfo();

				Respawn_Start();			
			}
			return;
		}
	}

	if (m_pIntroCamera && m_fIntroCamTime <= gpGlobals->time)
		MoveToNextIntroCamera();
}

void CBasePlayer::Disappear(void)
{
	if (m_pTank)
	{
		m_pTank->Use(this, this, USE_OFF, 0);
		m_pTank = NULL;
	}

	CSound *pSound = CSoundEnt::SoundPointerForIndex(CSoundEnt::ClientSoundIndex(edict()));

	if (pSound)
		pSound->Reset();

	m_fSequenceFinished = TRUE;
	pev->modelindex = m_ulModelIndexPlayer;
	pev->view_ofs = Vector(0, 0, -8);
	pev->deadflag = DEAD_DYING;
	pev->solid = SOLID_NOT;
	pev->flags &= FL_ONGROUND;

	SetSuitUpdate(NULL, FALSE, 0);

	m_iClientHealth = 0;
	MESSAGE_BEGIN(MSG_ONE, gmsgHealth, NULL, pev);
	WRITE_SHORT(m_iClientHealth);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ONE, gmsgCurWeapon, NULL, pev);
	WRITE_BYTE(0);
	WRITE_BYTE(0xFF);
	WRITE_BYTE(0xFF);
	MESSAGE_END();

	m_iFOV = m_iClientFOV = 0;

	MESSAGE_BEGIN(MSG_ONE, gmsgSetFOV, NULL, pev);
	WRITE_BYTE(0);
	MESSAGE_END();

	g_pGameRules->CheckWinConditions();
	m_bNotKilled = FALSE;

	SetThink(&CBasePlayer::PlayerDeathThink);
	pev->nextthink = gpGlobals->time + 0.1;
	pev->angles.x = 0;
	pev->angles.z = 0;
}

void CBasePlayer::PlayerDeathThink(void)
{
	if (m_iJoiningState != JOINED)
		return;

	if (FBitSet(pev->flags, FL_ONGROUND))
	{
		float flForward = pev->velocity.Length() - 20;

		if (flForward <= 0)
			pev->velocity = g_vecZero;
		else
			pev->velocity = flForward * pev->velocity.Normalize();
	}

	if (HasWeapons())
		PackDeadPlayerItems();

	if (pev->modelindex && !m_fSequenceFinished && pev->deadflag == DEAD_DYING)
	{
		StudioFrameAdvance();
		return;
	}

	if (pev->movetype != MOVETYPE_NONE && FBitSet(pev->flags, FL_ONGROUND))
		pev->movetype = MOVETYPE_NONE;

	if (pev->deadflag == DEAD_DYING)
	{
		m_fDeadTime = gpGlobals->time;
		pev->deadflag = DEAD_DEAD;
		PostDeath();//ckf3 added
	}

	StopAnimation();

	pev->effects |= EF_NOINTERP;
	pev->framerate = 0;

	BOOL fAnyButtonDown = (pev->button & ~IN_SCORE);

	if (g_pGameRules->IsMultiplayer())
	{
		if (gpGlobals->time > m_fDeadTime + 3)
		{
			if (!(m_afPhysicsFlags & PFLAG_OBSERVER))
			{
				SpawnClientSideCorpse();

				if (pev->view_ofs != g_vecZero)
					StartObserver(pev->origin, pev->angles);
			}
		}
	}

	if (pev->deadflag == DEAD_DEAD && m_iTeam != TEAM_UNASSIGNED && m_iTeam != TEAM_SPECTATOR)
	{
		if (fAnyButtonDown)
			return;

		if (g_pGameRules->FPlayerCanRespawn(this))
		{
			pev->deadflag = DEAD_RESPAWNABLE;

			if (g_pGameRules->IsMultiplayer())
				g_pGameRules->CheckWinConditions();
		}

		pev->nextthink = gpGlobals->time + 0.1;
	}
	else if (pev->deadflag == DEAD_RESPAWNABLE)
	{
		respawn(pev, FALSE);
		pev->button = 0;
		pev->nextthink = -1;
	}
}

void CBasePlayer::RoundRespawn(void)
{
	m_canSwitchObserverModes = true;

	if (m_iJoiningState == JOINED)
	{
		respawn(pev, FALSE);
		pev->button = 0;
		pev->nextthink = -1;
	}
}

void CBasePlayer::StartObserver(Vector vecPosition, Vector vecViewAngle)
{
	MESSAGE_BEGIN(MSG_PAS, SVC_TEMPENTITY, pev->origin);
	WRITE_BYTE(TE_KILLPLAYERATTACHMENTS);
	WRITE_BYTE(ENTINDEX(edict()));
	MESSAGE_END();

	if (m_pActiveItem)
		m_pActiveItem->Holster();

	if (m_pTank)
	{
		m_pTank->Use(this, this, USE_OFF, 0);
		m_pTank = NULL;
	}

	SetSuitUpdate(NULL, FALSE, 0);

	MESSAGE_BEGIN(MSG_ONE, gmsgCurWeapon, NULL, pev);
	WRITE_BYTE(0);
	WRITE_BYTE(0xFF);
	WRITE_BYTE(0xFF);
	MESSAGE_END();

	pev->fov = m_iFOV = m_iClientFOV = 0;

	MESSAGE_BEGIN(MSG_ONE, gmsgSetFOV, NULL, pev);
	WRITE_BYTE(0);
	MESSAGE_END();

	m_iHideHUD = (HIDEHUD_WEAPONS | HIDEHUD_HEALTH);
	m_afPhysicsFlags |= PFLAG_OBSERVER;
	pev->effects = EF_NODRAW;
	pev->view_ofs = g_vecZero;
	pev->angles = pev->v_angle = vecViewAngle;
	pev->fixangle = TRUE;
	pev->solid = SOLID_NOT;
	pev->takedamage = DAMAGE_NO;
	pev->movetype = MOVETYPE_NONE;

	UTIL_SetOrigin(pev, vecPosition);

	m_afPhysicsFlags &= ~PFLAG_DUCKING;
	pev->flags &= ~FL_DUCKING;
	pev->health = 1;
	m_iObserverC4State = 0;
	m_bObserverHasDefuser = FALSE;
	m_iObserverWeapon = 0;
	m_flNextObserverInput = 0;
	pev->iuser1 = OBS_NONE;

	Observer_SetMode(m_iObserverLastMode);
	ResetMaxSpeed();

	MESSAGE_BEGIN(MSG_ALL, gmsgSpectator);
	WRITE_BYTE(ENTINDEX(edict()));
	WRITE_BYTE(1);
	MESSAGE_END();
}

#define PLAYER_SEARCH_RADIUS 64.0

void CBasePlayer::PlayerUse(void)
{
	if (!((pev->button | m_afButtonPressed | m_afButtonReleased) & IN_USE))
		return;

	if (m_afButtonPressed & IN_USE)
	{
		if (m_pTank)
		{
			m_pTank->Use(this, this, USE_OFF, 0);
			m_pTank = NULL;
			return;
		}

		if (m_afPhysicsFlags & PFLAG_ONTRAIN)
		{
			m_iTrain = TRAIN_NEW | TRAIN_OFF;
			m_afPhysicsFlags &= ~PFLAG_ONTRAIN;
			CBaseEntity *pTrain = Instance(pev->groundentity);

			if (pTrain && pTrain->Classify() == CLASS_VEHICLE)
				((CFuncVehicle *)pTrain)->m_pDriver = NULL;

			return;
		}

		CBaseEntity *pTrain = Instance(pev->groundentity);

		if (pTrain && !(pev->button & IN_JUMP) && FBitSet(pev->flags, FL_ONGROUND) && (pTrain->ObjectCaps() & FCAP_DIRECTIONAL_USE) && pTrain->OnControls(pev))
		{
			m_afPhysicsFlags |= PFLAG_ONTRAIN;
			m_iTrain = TrainSpeed(pTrain->pev->speed, pTrain->pev->impulse);

			if (pTrain->Classify() == CLASS_VEHICLE)
			{
				EMIT_SOUND(ENT(pev), CHAN_ITEM, "plats/vehicle_ignition.wav", 0.8, ATTN_NORM);
				((CFuncVehicle *)pTrain)->m_pDriver = this;
			}
			else
				EMIT_SOUND(ENT(pev), CHAN_ITEM, "plats/train_use1.wav", 0.8, ATTN_NORM);

			return;
		}
	}

	CBaseEntity *pObject = NULL;
	CBaseEntity *pClosest = NULL;
	float flMaxDot = VIEW_FIELD_NARROW;

	UTIL_MakeVectors(pev->v_angle);

	while ((pObject = UTIL_FindEntityInSphere(pObject, pev->origin, PLAYER_SEARCH_RADIUS)) != NULL)
	{
		if (pObject->ObjectCaps() & (FCAP_IMPULSE_USE | FCAP_CONTINUOUS_USE | FCAP_ONOFF_USE))
		{
			Vector vecLOS = VecBModelOrigin(pObject->pev) - (pev->origin + pev->view_ofs);
			vecLOS = UTIL_ClampVectorToBox(vecLOS, pObject->pev->size * 0.5);
			float flDot = DotProduct(vecLOS, gpGlobals->v_forward);

			if (flDot > flMaxDot)
			{
				flMaxDot = flDot;
				pClosest = pObject;
			}
		}
	}

	pObject = pClosest;

	if (pObject)
	{
		int caps = pObject->ObjectCaps();

		if (m_afButtonPressed & IN_USE)
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "common/wpn_select.wav", 0.4, ATTN_NORM);

		if (((pev->button & IN_USE) && (caps & FCAP_CONTINUOUS_USE)) || ((m_afButtonPressed & IN_USE) && (caps & (FCAP_IMPULSE_USE | FCAP_ONOFF_USE))))
		{
			if (caps & FCAP_CONTINUOUS_USE)
				m_afPhysicsFlags |= PFLAG_USING;

			pObject->Use(this, this, USE_SET, 1);
			return;
		}

		if ((m_afButtonReleased & IN_USE) && (pObject->ObjectCaps() & FCAP_ONOFF_USE))
			pObject->Use(this, this, USE_SET, 0);
	}
	else if (m_afButtonPressed & IN_USE)
		EMIT_SOUND(ENT(pev), CHAN_ITEM, "common/wpn_denyselect.wav", 0.4, ATTN_NORM);
}

void CBasePlayer::HostageUsed(void)
{
	if (m_flDisplayHistory & DHF_HOSTAGE_FOLLOWED)
		return;

	if (m_iTeam == TEAM_TERRORIST)
		HintMessage("#Hint_use_hostage_to_stop_him");
	else if (m_iTeam == TEAM_CT)
		HintMessage("#Hint_lead_hostage_to_rescue_point");

	m_flDisplayHistory |= DHF_HOSTAGE_FOLLOWED;
}

void CBasePlayer::Jump(void)
{
	if (pev->flags & FL_WATERJUMP)
		return;

	if (pev->waterlevel >= 2)
		return;

	if (!(pev->button & IN_JUMP) || (pev->oldbuttons & IN_JUMP))//if (!(m_afButtonPressed & IN_JUMP))
		return;

	if ((!m_iMultiJumpMax || !(m_iMultiJumpCurrent < m_iMultiJumpMax)) && (!(pev->flags & FL_ONGROUND) || !pev->groundentity))
		return;

	if ((pev->flags & FL_ONGROUND) && pev->groundentity)
		SetAnimation(PLAYER_JUMP);
	else
	{
		if (!m_bMultiJump)
		{
			m_bMultiJump = true;
			pev->flags |= FL_MULTIJUMP;
		}
		m_iMultiJumpCurrent++;
		SetAnimation(PLAYER_SUPERJUMP);
		EMIT_SOUND(ENT(pev), CHAN_STATIC, "CKF_III/multijump.wav", VOL_NORM, ATTN_NORM);

		MESSAGE_BEGIN(MSG_BROADCAST, gmsgDrawFX);
		WRITE_BYTE(FX_MULTIJUMPTRAIL);
		WRITE_SHORT(entindex());
		MESSAGE_END();
	}

	entvars_t *pevGround = VARS(pev->groundentity);

	if (pevGround)
	{
		if (pevGround->flags & FL_CONVEYOR)
			pev->velocity = pev->velocity + pev->basevelocity;

		if (FClassnameIs(pevGround, "func_tracktrain") || FClassnameIs(pevGround, "func_train") || FClassnameIs(pevGround, "func_vehicle"))
			pev->velocity = pevGround->velocity + pev->velocity;
	}
}

void FixPlayerCrouchStuck(edict_t *pPlayer)
{
	for (int i = 0; i < 18; i++)
	{
		TraceResult trace;
		UTIL_TraceHull(pPlayer->v.origin, pPlayer->v.origin, dont_ignore_monsters, head_hull, pPlayer, &trace);

		if (!trace.fStartSolid)
			break;

		pPlayer->v.origin.z++;
	}
}

void CBasePlayer::Duck(void)
{
	if (pev->button & IN_DUCK)
		SetAnimation(PLAYER_WALK);
}

int CBasePlayer::Classify(void)
{
	return CLASS_PLAYER;
}

void CBasePlayer::AddPoints(float score, BOOL bAllowNegativeScore)
{
	if (score < 0 && !bAllowNegativeScore)
	{
		if (pev->frags < 0)
			return;

		if (-score > pev->frags)
			score = -pev->frags;
	}

	int iOldFrags = pev->frags;

	pev->frags += score;

	if(iOldFrags != (int)pev->frags)
	{
		SendScoreInfo();
	}
}

void CBasePlayer::AddPointsToTeam(int score, BOOL bAllowNegativeScore)
{
	int index = entindex();

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer *pOther = (CBasePlayer *)UTIL_PlayerByIndex(i);

		if (pOther && i != index)
		{
			if (g_pGameRules->PlayerRelationship(this, pOther) == GR_TEAMMATE)
				pOther->AddPoints(score, bAllowNegativeScore);
		}
	}
}

#define HUDSTATUS_DISABLE (1<<0)
#define HUDSTATUS_TEAMMATE (1<<1)
#define HUDSTATUS_PLAYER (1<<2)
#define HUDSTATUS_BUILDABLE (1<<3)//max == (1<<7)

//void CBasePlayer::UpdateStatusBar(void)
//{
//	BOOL bSeePlayer = FALSE;
//	BOOL bSeeBuildable = FALSE;
//	BOOL bSeeTeammate = FALSE;
//	CBasePlayer *pPlayer = NULL;
//	CBaseEntity *pEntity = NULL;
//	CBaseBuildable *pBuild = NULL;
//	edict_t *peMe = edict();
//	if (m_hObserverTarget)
//	{
//		//数据显示为观察目标的
//		//return;
//	}
//	else
//	{
//		int iPenetration = 5;
//		float range = (pev->flags & FL_SPECTATOR) ? MAX_SPECTATOR_ID_RANGE : MAX_ID_RANGE;
//		TraceResult tr;
//		UTIL_MakeVectors(pev->v_angle);
//		Vector vecStart = EyePosition();
//		Vector vecEnd = vecStart + (gpGlobals->v_forward * range);
//		edict_t *pEnt = peMe;
//		while(iPenetration)
//		{
//			iPenetration --;
//			UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, pEnt, &tr);
//			if(tr.flFraction != 1)
//			{
//				if(!FNullEnt(tr.pHit) && tr.pHit != peMe)
//				{
//					pEnt = tr.pHit;
//					pEntity = Instance(tr.pHit);
//
//					if (pEntity->Classify() == CLASS_PLAYER)
//					{
//						pPlayer = (CBasePlayer *)pEntity;
//						if(pPlayer->m_iCloak == CLOAK_YES)
//							continue;
//						bSeePlayer = TRUE;
//
//						if(m_iTeam == pPlayer->m_iTeam)
//							bSeeTeammate = TRUE;
//						if(m_iClass == CLASS_SPY)
//							bSeeTeammate = TRUE;
//						if(pPlayer->m_iDisguise == DISGUISE_YES && pPlayer->m_iDisguiseTeam == m_iTeam)
//							bSeeTeammate = TRUE;
//						if(IsObserver())
//							bSeeTeammate = TRUE;
//						break;
//					}
//					else if (pEntity->Classify() == CLASS_BUILDABLE)
//					{
//						pBuild = (CBaseBuildable *)pEntity;
//						pPlayer = pBuild->m_pPlayer;
//						if(!pPlayer)
//							continue;
//						bSeeBuildable = TRUE;
//
//						if(m_iTeam == pBuild->m_iTeam)
//							bSeeTeammate = TRUE;
//						if(m_iClass == CLASS_SPY)
//							bSeeTeammate = TRUE;
//						if(IsObserver())
//							bSeeTeammate = TRUE;
//						break;
//					}
//					else
//						iPenetration = 0;
//				}
//				vecStart = vecStart + 32*gpGlobals->v_forward;
//			}
//			else
//				iPenetration = 0;
//		}
//	}
//
//	int iWriteFlags = 0;
//	int iWriteEntIndex = 0;
//	int iWriteEntData = 0;
//
//	int iBuildClass = 0;
//	int iBuildLevel = 1;
//	int iBuildUpgrade = 0;
//
//	if(bSeePlayer)
//	{
//		iWriteFlags = HUDSTATUS_PLAYER;
//		iWriteEntIndex = pPlayer->entindex();
//		if(bSeeTeammate)
//		{
//			iWriteFlags |= HUDSTATUS_TEAMMATE;
//			if(pPlayer->m_iDisguise == DISGUISE_YES && pPlayer->m_iDisguiseTeam == m_iTeam && pPlayer->m_iTeam != m_iTeam)
//			{
//				if(pPlayer->m_iDisguiseWeaponID)
//					iWriteEntData = pPlayer->m_iDisguiseHealth * pPlayer->m_iDisguiseMaxHealth * pPlayer->m_iDisguiseWeaponID;
//				else
//					iWriteEntData = pPlayer->m_iDisguiseHealth * pPlayer->m_iDisguiseMaxHealth;
//			}
//			else
//			{
//				if(pPlayer->m_pActiveItem)
//					iWriteEntData = pPlayer->pev->health * pPlayer->pev->max_health * pPlayer->m_pActiveItem->m_iId;
//				else
//					iWriteEntData = pPlayer->pev->health * pPlayer->pev->max_health;
//			}
//		}
//	}
//	else if(bSeeBuildable)
//	{
//		iWriteFlags = HUDSTATUS_BUILDABLE;
//		iWriteEntIndex = pPlayer->entindex();
//		iBuildClass = pBuild->GetBuildClass();
//		iBuildLevel = pBuild->GetBuildLevel();
//		if(bSeeTeammate)
//		{
//			iWriteFlags |= HUDSTATUS_TEAMMATE;			
//			iBuildUpgrade = pBuild->GetBuildUpgrade();
//			if(iBuildLevel < 3)
//				iWriteEntData = (pBuild->pev->health + pBuild->pev->max_health) * (iBuildUpgrade + iBuildClass);
//			else
//				iWriteEntData = (pBuild->pev->health + pBuild->pev->max_health) * iBuildClass;
//
//			if(iBuildClass == BUILDABLE_ENTRANCE)
//			{
//				CBuildTeleporter *pTele = (CBuildTeleporter *)pBuild;
//				float flChargeRate = pTele->GetChargeRate();
//				iWriteEntData *= max(flChargeRate, 1);
//			}
//		}
//		else
//			iWriteEntData = iBuildClass;
//	}
//	else
//	{
//		iWriteFlags = HUDSTATUS_DISABLE;
//	}
//
//	if(iWriteFlags != m_iClientSBarFlags || iWriteEntIndex != m_iClientSBarEntIndex || iWriteEntData != m_iClientSBarEntData)
//	{
//		m_iClientSBarFlags = iWriteFlags;
//		m_iClientSBarEntIndex = iWriteEntIndex;
//		m_iClientSBarEntData = iWriteEntData;
//	}
//	else
//	{
//		return;
//	}
//
//	if(bSeePlayer && bSeeTeammate)
//	{
//		MESSAGE_BEGIN(MSG_ONE, gmsgHUDStatus, NULL, pev);
//		WRITE_BYTE(iWriteFlags);
//		WRITE_BYTE(iWriteEntIndex);
//		if(pPlayer->m_iDisguise == DISGUISE_YES && pPlayer->m_iDisguiseTeam == m_iTeam && pPlayer->m_iTeam != m_iTeam)
//		{
//			WRITE_SHORT(pPlayer->m_iDisguiseHealth);
//			WRITE_SHORT(pPlayer->m_iDisguiseMaxHealth);
//			if(pPlayer->m_iDisguiseWeaponID)
//				WRITE_BYTE(pPlayer->m_iDisguiseWeaponID);
//			else
//				WRITE_BYTE(0);
//		}
//		else
//		{
//			WRITE_SHORT(pPlayer->pev->health);
//			WRITE_SHORT(pPlayer->pev->max_health);
//			if(pPlayer->m_pActiveItem)
//				WRITE_BYTE(pPlayer->m_pActiveItem->m_iId);
//			else
//				WRITE_BYTE(0);
//		}
//		MESSAGE_END();
//	}
//	else if(bSeePlayer && !bSeeTeammate)
//	{
//		MESSAGE_BEGIN(MSG_ONE, gmsgHUDStatus, NULL, pev);
//		WRITE_BYTE(iWriteFlags);
//		WRITE_BYTE(iWriteEntIndex);
//		MESSAGE_END();
//	}
//	else if(bSeeBuildable && bSeeTeammate)
//	{
//		MESSAGE_BEGIN(MSG_ONE, gmsgHUDStatus, NULL, pev);
//		WRITE_BYTE(iWriteFlags);
//		WRITE_BYTE(iWriteEntIndex);
//		WRITE_BYTE(iBuildClass);
//		WRITE_BYTE(iBuildLevel);
//		WRITE_SHORT(pBuild->pev->health);
//		WRITE_SHORT(pBuild->pev->max_health);		
//		if(iBuildLevel < 3)
//			WRITE_BYTE(iBuildUpgrade);
//		if(iBuildClass == BUILDABLE_ENTRANCE)
//		{
//			CBuildTeleporter *pTele = (CBuildTeleporter *)pBuild;
//			WRITE_COORD(pTele->m_flCharge);
//			WRITE_COORD(pTele->GetChargeRate());
//		}
//		MESSAGE_END();
//	}
//	else if(bSeeBuildable && !bSeeTeammate)
//	{
//		MESSAGE_BEGIN(MSG_ONE, gmsgHUDStatus, NULL, pev);
//		WRITE_BYTE(iWriteFlags);
//		WRITE_BYTE(iWriteEntIndex);
//		WRITE_BYTE(iBuildClass);
//		WRITE_BYTE(iBuildLevel);
//		MESSAGE_END();
//	}
//	else if(iWriteFlags & HUDSTATUS_DISABLE)
//	{
//		MESSAGE_BEGIN(MSG_ONE, gmsgHUDStatus, NULL, pev);
//		WRITE_BYTE(iWriteFlags);
//		MESSAGE_END();
//	}
//}

void CBasePlayer::PreThink(void)
{
	int buttonsChanged = pev->button ^ m_afButtonLast;

	if (pev->button != m_afButtonLast)
		m_fLastMovement = gpGlobals->time;

	m_afButtonPressed = buttonsChanged & pev->button;
	m_afButtonReleased = buttonsChanged & ~pev->button;
	m_hintMessageQueue.Update(this);
	g_pGameRules->PlayerThink(this);

	if (g_fGameOver)
		return;

	if (m_iJoiningState != JOINED)
		JoiningThink();

	/*if (m_bMissionBriefing == TRUE)
	{
		if (m_afButtonPressed & (IN_ATTACK | IN_ATTACK2))
		{
			m_afButtonPressed &= ~(IN_ATTACK | IN_ATTACK2);
			RemoveLevelText();
			m_bMissionBriefing = FALSE;
		}
	}*/

	UTIL_MakeVectors(pev->v_angle);
	ItemPreFrame();
	WaterMove();

	if (FBitSet(pev->flags, FL_ONGROUND))
	{
		if (m_flVelocityModifier < 1)
		{
			m_flVelocityModifier += 0.01;
			pev->velocity = pev->velocity * m_flVelocityModifier;
		}

		if (m_flVelocityModifier > 1)
			m_flVelocityModifier = 1;
	}

	if (m_flIdleCheckTime <= gpGlobals->time || !m_flIdleCheckTime)
	{
		m_flIdleCheckTime = gpGlobals->time + 5;

		if (gpGlobals->time - m_fLastMovement > g_pGameRules->m_fMaxIdlePeriod)
		{
			if (CVAR_GET_FLOAT("mp_autokick"))
			{
				UTIL_ClientPrintAll(HUD_PRINTCONSOLE, "#Game_idle_kick", STRING(pev->netname));
				SERVER_COMMAND(UTIL_VarArgs("kick %s\n", STRING(pev->netname)));
				m_fLastMovement = gpGlobals->time;
			}
		}
	}

	CritChance_Think();
	CritBuff_Think();
	Respawn_Think();

	Condition_Think();

	//flush hit damage messages
	FlushHitDamage();
	
	if(IsAlive())
	{
		Cloak_Think();
		Disguise_Think();
		Health_Think();
		Uber_Think();
	}

	UpdateClientData();
	CheckTimeBasedDamage();
	CheckSuitUpdate();

	if (m_afPhysicsFlags & PFLAG_ONTRAIN)
		pev->flags |= FL_ONTRAIN;
	else
		pev->flags &= ~FL_ONTRAIN;

	if (IsObserver() && (m_afPhysicsFlags & PFLAG_OBSERVER))
	{
		Observer_HandleButtons();
		Observer_CheckTarget();
		Observer_CheckProperties();
		return;
	}

	if (pev->deadflag >= DEAD_DYING && pev->deadflag != DEAD_RESPAWNABLE)
	{
		PlayerDeathThink();
		return;
	}

	CBaseEntity *pGroundEntity = Instance(pev->groundentity);

	if (pGroundEntity && pGroundEntity->Classify() == CLASS_VEHICLE)
		pev->iuser4 = 1;
	else
		pev->iuser4 = 0;

	if (m_afPhysicsFlags & PFLAG_ONTRAIN)
	{
		CBaseEntity *pTrain = Instance(pev->groundentity);

		if (!pTrain)
		{
			TraceResult trainTrace;
			UTIL_TraceLine(pev->origin, pev->origin + Vector(0, 0, -38), ignore_monsters, ENT(pev), &trainTrace);

			if (trainTrace.flFraction != 1 || trainTrace.pHit)
				pTrain = Instance(trainTrace.pHit);

			if (!pTrain || !(pTrain->ObjectCaps() & FCAP_DIRECTIONAL_USE) || !pTrain->OnControls(pev))
			{
				m_iTrain = TRAIN_NEW | TRAIN_OFF;
				m_afPhysicsFlags &= ~PFLAG_ONTRAIN;
				((CFuncVehicle *)pTrain)->m_pDriver = NULL;
				return;
			}
		}
		else if (!FBitSet(pev->flags, FL_ONGROUND) || (pTrain->pev->spawnflags & SF_TRACKTRAIN_NOCONTROL))
		{
			m_iTrain = TRAIN_NEW | TRAIN_OFF;
			m_afPhysicsFlags &= ~PFLAG_ONTRAIN;
			((CFuncVehicle *)pTrain)->m_pDriver = NULL;
			return;
		}

		float vel = 0;
		pev->velocity = g_vecZero;

		if (pTrain->Classify() == CLASS_VEHICLE)
		{
			if (pev->button & IN_FORWARD)
			{
				vel = 1;
				pTrain->Use(this, this, USE_SET, vel);
			}

			if (pev->button & IN_BACK)
			{
				vel = -1;
				pTrain->Use(this, this, USE_SET, vel);
			}

			if (pev->button & IN_MOVELEFT)
			{
				vel = 20;
				pTrain->Use(this, this, USE_SET, vel);
			}

			if (pev->button & IN_MOVERIGHT)
			{
				vel = 30;
				pTrain->Use(this, this, USE_SET, vel);
			}
		}
		else
		{
			if (m_afButtonPressed & IN_FORWARD)
			{
				vel = 1;
				pTrain->Use(this, this, USE_SET, vel);
			}
			else if (m_afButtonPressed & IN_BACK)
			{
				vel = -1;
				pTrain->Use(this, this, USE_SET, vel);
			}
		}

		if (vel)
		{
			m_iTrain = TrainSpeed(pTrain->pev->speed, pTrain->pev->impulse);
			m_iTrain |= TRAIN_ACTIVE | TRAIN_NEW;
		}
	}
	else if (m_iTrain & TRAIN_ACTIVE)
		m_iTrain = TRAIN_NEW;

	Jump();//multi jump predict

	if (m_bMultiJump && (pev->flags & FL_ONGROUND))
	{
		MESSAGE_BEGIN(MSG_BROADCAST, gmsgDrawFX);
		WRITE_BYTE(FX_KILLTRAIL);
		WRITE_SHORT(entindex());
		MESSAGE_END();
		m_iMultiJumpCurrent = 0;
		m_bMultiJump = false;
		pev->flags &= ~FL_MULTIJUMP;
	}

	if (m_iClass == CLASS_SOLDIER)
		m_iRocketJump = !(pev->flags & FL_ONGROUND);

	if ((pev->button & IN_DUCK) || FBitSet(pev->flags, FL_DUCKING) || (m_afPhysicsFlags & PFLAG_DUCKING))
		Duck();

	if (!FBitSet(pev->flags, FL_ONGROUND))
		m_flFallVelocity = -pev->velocity.z;

	m_hEnemy = NULL;

	if (m_afPhysicsFlags & PFLAG_ONBARNACLE)
		pev->velocity = g_vecZero;

	if (!(m_flDisplayHistory & DHF_ROUND_STARTED))
	{
		//HintMessage("#Hint_press_buy_to_purchase");
		m_flDisplayHistory |= DHF_ROUND_STARTED;
	}
}

void CBasePlayer::CheckTimeBasedDamage(void)
{
	BYTE bDuration = 0;

	if (!(m_bitsDamageType & DMG_TIMEBASED))
		return;

	if (abs(gpGlobals->time - m_tbdPrev) < 2)
		return;

	m_tbdPrev = gpGlobals->time;

	for (int i = 0; i < CDMG_TIMEBASED; i++)
	{
		if (m_bitsDamageType & (DMG_PARALYZE << i))
		{
			switch (i)
			{
				case itbd_Paralyze: bDuration = PARALYZE_DURATION; break;
				case itbd_NerveGas: bDuration = NERVEGAS_DURATION; break;

				case itbd_Poison:
				{
					TakeDamage(pev, pev, POISON_DAMAGE, DMG_GENERIC, 0);
					bDuration = POISON_DURATION;
					break;
				}

				case itbd_DrownRecover:
				{
					if (m_idrowndmg > m_idrownrestored)
					{
						int idif = min(m_idrowndmg - m_idrownrestored, 10);
						TakeHealth(idif, DMG_GENERIC);
						m_idrownrestored += idif;
					}

					bDuration = 4;
					break;
				}

				case itbd_Radiation: bDuration = RADIATION_DURATION; break;
				case itbd_Acid: bDuration = ACID_DURATION; break;
				case itbd_SlowBurn: bDuration = SLOWBURN_DURATION; break;
				case itbd_SlowFreeze: bDuration = SLOWFREEZE_DURATION; break;
				default: bDuration = 0;
			}

			if (m_rgbTimeBasedDamage[i])
			{
				if ((i == itbd_NerveGas && m_rgbTimeBasedDamage[i] < NERVEGAS_DURATION) || (i == itbd_Poison && m_rgbTimeBasedDamage[i] < POISON_DURATION))
				{
					if (m_rgItems[ITEM_ANTIDOTE])
					{
						m_rgbTimeBasedDamage[i] = 0;
						m_rgItems[ITEM_ANTIDOTE]--;
						SetSuitUpdate("!HEV_HEAL4", FALSE, SUIT_REPEAT_OK);
					}
				}

				if (!m_rgbTimeBasedDamage[i] || --m_rgbTimeBasedDamage[i] == 0)
				{
					m_rgbTimeBasedDamage[i] = 0;
					m_bitsDamageType &= ~(DMG_PARALYZE << i);
				}
			}
			else
				m_rgbTimeBasedDamage[i] = bDuration;
		}
	}
}

#define GEIGERDELAY 0.25

void CBasePlayer::UpdateGeigerCounter(void)
{
	if (gpGlobals->time < m_flgeigerDelay)
		return;

	m_flgeigerDelay = gpGlobals->time + GEIGERDELAY;
	byte range = (byte)(m_flgeigerRange / 4);

	if (range != m_igeigerRangePrev)
	{
		m_igeigerRangePrev = range;
		MESSAGE_BEGIN(MSG_ONE, gmsgGeigerRange, NULL, pev);
		WRITE_BYTE(range);
		MESSAGE_END();
	}

	if (!RANDOM_LONG(0, 3))
		m_flgeigerRange = 1000;
}

#define SUITUPDATETIME 3.5
#define SUITFIRSTUPDATETIME 0.1

void CBasePlayer::CheckSuitUpdate(void)
{
	int isentence = 0;
	int isearch = m_iSuitPlayNext;

	if (!(pev->weapons & (1 << WEAPON_SUIT)))
		return;

	UpdateGeigerCounter();

	if (g_pGameRules->IsMultiplayer())
		return;

	if (gpGlobals->time >= m_flSuitUpdate && m_flSuitUpdate > 0)
	{
		for (int i = 0; i < CSUITPLAYLIST; i++)
		{
			if (isentence = m_rgSuitPlayList[isearch])
				break;

			if (++isearch == CSUITPLAYLIST)
				isearch = 0;
		}

		if (isentence)
		{
			m_rgSuitPlayList[isearch] = 0;

			if (isentence > 0)
			{
				char sentence[CBSENTENCENAME_MAX + 1];
				strcpy(sentence, "!");
				strcat(sentence, gszallsentencenames[isentence]);
				EMIT_SOUND_SUIT(ENT(pev), sentence);
			}
			else
				EMIT_GROUPID_SUIT(ENT(pev), -isentence);

			m_flSuitUpdate = gpGlobals->time + SUITUPDATETIME;
		}
		else
			m_flSuitUpdate = 0;
	}
}

void CBasePlayer::SetSuitUpdate(char *name, int fgroup, int iNoRepeatTime)
{
}

void CBasePlayer::CheckPowerups()
{
	//2015-9-20 this might be a potential bug

	/*if (pev->health > 0)
	{
		pev->modelindex = m_ulModelIndexPlayer;
	}*/
}

void CBasePlayer::UpdatePlayerSound(void)
{
	int iBodyVolume;
	CSound *pSound = CSoundEnt::SoundPointerForIndex(CSoundEnt::ClientSoundIndex(edict()));

	if (!pSound)
	{
		ALERT(at_console, "Client lost reserved sound!\n");
		return;
	}

	pSound->m_iType = bits_SOUND_NONE;

	if (FBitSet(pev->flags, FL_ONGROUND))
	{
		iBodyVolume = pev->velocity.Length();

		if (iBodyVolume> 512)
			iBodyVolume = 512;
	}
	else
		iBodyVolume = 0;

	if (pev->button & IN_JUMP)
		iBodyVolume += 100;

	if (m_iWeaponVolume > iBodyVolume)
	{
		m_iTargetVolume = m_iWeaponVolume;
		pSound->m_iType |= bits_SOUND_COMBAT;
	}
	else
		m_iTargetVolume = iBodyVolume;

	m_iWeaponVolume -= 250 * gpGlobals->frametime;
	int iVolume = pSound->m_iVolume;

	if (m_iTargetVolume > iVolume)
	{
		iVolume = m_iTargetVolume;
	}
	else if (iVolume > m_iTargetVolume)
	{
		iVolume -= 250 * gpGlobals->frametime;

		if (iVolume < m_iTargetVolume)
			iVolume = 0;
	}

	if (m_fNoPlayerSound)
		iVolume = 0;

	if (gpGlobals->time > m_flStopExtraSoundTime)
		m_iExtraSoundTypes = 0;

	if (pSound)
	{
		pSound->m_vecOrigin = pev->origin;
		pSound->m_iVolume = iVolume;
		pSound->m_iType |= (bits_SOUND_PLAYER | m_iExtraSoundTypes);
	}

	m_iWeaponFlash -= 256 * gpGlobals->frametime;

	if (m_iWeaponFlash < 0)
		m_iWeaponFlash = 0;

	UTIL_MakeVectors(pev->angles);
	gpGlobals->v_forward.z = 0;
}

void CBasePlayer::PostThink(void)
{
	if (g_fGameOver)
		goto pt_end;

	if (!IsAlive())
		goto pt_end;

	if (m_pTank)
	{
		if (!m_pTank->OnControls(pev) || pev->weaponmodel)
		{
			m_pTank->Use(this, this, USE_OFF, 0);
			m_pTank = NULL;
		}
		else
			m_pTank->Use(this, this, USE_SET, 2);
	}

	ItemPostFrame();

	if ((pev->flags & FL_ONGROUND) && pev->health > 0 && m_flFallVelocity >= PLAYER_FALL_PUNCH_THRESHHOLD)
	{
		if (pev->watertype != CONTENT_WATER)
		{
			if (m_flFallVelocity > PLAYER_MAX_SAFE_FALL_SPEED)
			{
				float flFallDamage = g_pGameRules->FlPlayerFallDamage(this);

				if (flFallDamage > pev->health)
					EMIT_SOUND(ENT(pev), CHAN_ITEM, "common/bodysplat.wav", VOL_NORM, ATTN_NORM);

				if (flFallDamage > 0)
				{
					TakeDamage(VARS(eoNullEntity), VARS(eoNullEntity), flFallDamage, DMG_FALL, 0);
					pev->punchangle.x = 0;
				}
			}
		}

		if (IsAlive())
			SetAnimation(PLAYER_WALK);
	}

	if (pev->flags & FL_ONGROUND)
	{
		if (m_flFallVelocity > 64 && !g_pGameRules->IsMultiplayer())
			CSoundEnt::InsertSound(bits_SOUND_PLAYER, pev->origin, m_flFallVelocity, 0.2);

		m_flFallVelocity = 0;
	}

	if (IsAlive())
	{
		if (pev->velocity.x || pev->velocity.y)
		{
			if ((pev->velocity.x || pev->velocity.y) && FBitSet(pev->flags, FL_ONGROUND))
				SetAnimation(PLAYER_WALK);
			else if (pev->waterlevel > 1)
				SetAnimation(PLAYER_WALK);
		}
		else if (pev->gaitsequence != ACT_FLY)
			SetAnimation(PLAYER_IDLE);
	}

	StudioFrameAdvance();
	CheckPowerups();
	UpdatePlayerSound();

pt_end:
#ifdef CLIENT_WEAPONS
	for (int i = 0; i < MAX_ITEM_TYPES; i++)
	{
		if (!m_rgpPlayerItems[i])
			continue;

		CBasePlayerItem *pPlayerItem = m_rgpPlayerItems[i];

		while (pPlayerItem)
		{
			CBasePlayerWeapon *gun = (CBasePlayerWeapon *)pPlayerItem->GetWeaponPtr();

			if (gun && gun->UseDecrement())
			{
				gun->m_flNextPrimaryAttack = max(gun->m_flNextPrimaryAttack - gpGlobals->frametime, -1.0);
				gun->m_flNextSecondaryAttack = max(gun->m_flNextSecondaryAttack - gpGlobals->frametime, -0.001);
				gun->m_flDecreaseShotsFired = max(gun->m_flDecreaseShotsFired - gpGlobals->frametime, -0.001);
				gun->m_flNextReload = max(gun->m_flNextReload - gpGlobals->frametime, -0.001);
				if(gun->iItemSlot() == WEAPON_SLOT_MELEE)
				{
					gun->m_flMeleeAttack = max(gun->m_flMeleeAttack - gpGlobals->frametime, -0.001);
				}
				else if(gun->m_iId == WEAPON_MINIGUN)
				{
					CMinigun *minigun = (CMinigun *)gun;
					minigun->m_fSpin = max(minigun->m_fSpin - gpGlobals->frametime, -0.001);
				}
				else if(gun->m_iId == WEAPON_STICKYLAUNCHER)
				{
					CStickyLauncher *stickylauncher = (CStickyLauncher *)gun;
					stickylauncher->m_fChargeTimer = max(stickylauncher->m_fChargeTimer - gpGlobals->frametime, -0.001);
				}
				else if(gun->m_iId == WEAPON_SNIPERIFLE)
				{
					CSniperifle *sniperifle = (CSniperifle *)gun;
					sniperifle->m_fChargeTimer = max(sniperifle->m_fChargeTimer - gpGlobals->frametime, -0.001);
				}
				else if(gun->m_iId == WEAPON_FLAMETHROWER)
				{
					CFlamethrower *flamethrower = (CFlamethrower *)gun;
					flamethrower->m_flState = max(flamethrower->m_flState - gpGlobals->frametime, -0.001);
				}

				if (gun->m_flTimeWeaponIdle != 1000)
					gun->m_flTimeWeaponIdle = max(gun->m_flTimeWeaponIdle - gpGlobals->frametime, -0.001);
			}

			pPlayerItem = pPlayerItem->m_pNext;
		}
	}

	m_flNextAttack -= gpGlobals->frametime;

	if (m_flNextAttack < -0.001)
		m_flNextAttack = -0.001;
#endif

	m_afButtonLast = pev->button;
	m_iGaitSequence = pev->gaitsequence;
	StudioProcessGait();
}

BOOL IsSpawnPointValid(CBaseEntity *pPlayer, CBaseEntity *pSpot)
{
	CBaseEntity *ent = NULL;

	if (!pSpot->IsTriggered(pPlayer))
		return FALSE;

	if( ((CBaseDMStart *)pSpot)->m_iDisabled )
		return FALSE;

	if(!UTIL_IsMasterTriggered( pSpot->pev->netname, NULL ))
		return FALSE;

	while ((ent = UTIL_FindEntityInSphere(ent, pSpot->pev->origin, 64)) != NULL)
	{
		if (ent->IsPlayer() && ent != pPlayer)
			return FALSE;
	}

	return TRUE;
}

DLL_GLOBAL CBaseEntity *g_pLastSpawn;
DLL_GLOBAL CBaseEntity *g_pLastCTSpawn, *g_pLastTerroristSpawn;

inline int FNullEnt(CBaseEntity *ent) { return (!ent) || FNullEnt(ent->edict()); }

BOOL SelectSpawnSpot(CBaseEntity *pPlayer, const char *pEntClassName, CBaseEntity *&pSpot)
{
	edict_t *player = pPlayer->edict();
	pSpot = UTIL_FindEntityByClassname(pSpot, pEntClassName);

	if (FNullEnt(pSpot))
		pSpot = UTIL_FindEntityByClassname(pSpot, pEntClassName);

	CBaseEntity *pFirstSpot = pSpot;

	do
	{
		if (pSpot)
		{
			if (IsSpawnPointValid(pPlayer, pSpot))
			{
				if (pSpot->pev->origin == Vector(0, 0, 0))
				{
					pSpot = UTIL_FindEntityByClassname(pSpot, pEntClassName);
					continue;
				}

				return TRUE;
			}
		}

		pSpot = UTIL_FindEntityByClassname(pSpot, pEntClassName);
	}
	while (pSpot != pFirstSpot);

	if (!FNullEnt(pSpot))
	{
		CBaseEntity *ent = UTIL_FindEntityInSphere(NULL, pSpot->pev->origin, 64);

		while (ent)
		{
			if (ent->IsPlayer() && ent->edict() != player)
				ent->TakeDamage(VARS(INDEXENT(0)), VARS(INDEXENT(0)), 9999, DMG_ALWAYSGIB, 0);//hz fuck up

			ent = UTIL_FindEntityInSphere(ent, pSpot->pev->origin, 64);
		}

		return TRUE;
	}

	return FALSE;
}

edict_t *EntSelectSpawnPoint(CBaseEntity *pPlayer)
{
	CBaseEntity *pSpot;
	edict_t *player = pPlayer->edict();

	if (g_pGameRules->IsCoOp())
	{
		pSpot = UTIL_FindEntityByClassname(g_pLastSpawn, "info_player_coop");

		if (!FNullEnt(pSpot))
			goto ReturnSpot;

		pSpot = UTIL_FindEntityByClassname(g_pLastSpawn, "info_player_start");

		if (!FNullEnt(pSpot))
			goto ReturnSpot;
	}
	else if (g_pGameRules->IsDeathmatch() && ((CBasePlayer *)pPlayer)->m_iTeam == TEAM_CT)
	{
//CTSpawn://Unreferenced
		pSpot = g_pLastCTSpawn;

		if (SelectSpawnSpot(pPlayer, "info_player_start", pSpot))
			goto ReturnSpot;
	}
	else if (g_pGameRules->IsDeathmatch() && ((CBasePlayer *)pPlayer)->m_iTeam == TEAM_TERRORIST)
	{
		pSpot = g_pLastTerroristSpawn;

		if (SelectSpawnSpot(pPlayer, "info_player_deathmatch", pSpot))
			goto ReturnSpot;
	}

	if (FStringNull(gpGlobals->startspot) || !strlen(STRING(gpGlobals->startspot)))
	{
		pSpot = UTIL_FindEntityByClassname(NULL, "info_player_deathmatch");

		if (!FNullEnt(pSpot))
			goto ReturnSpot;
	}
	else
	{
		pSpot = UTIL_FindEntityByTargetname(NULL, STRING(gpGlobals->startspot));

		if (!FNullEnt(pSpot))
			goto ReturnSpot;
	}

ReturnSpot:
	if (FNullEnt(pSpot))
	{
		ALERT(at_error, "PutClientInServer: no info_player_start on level");
		return INDEXENT(0);
	}

	if (((CBasePlayer *)pPlayer)->m_iTeam == TEAM_TERRORIST)
		g_pLastTerroristSpawn = pSpot;
	else
		g_pLastCTSpawn = pSpot;

	return pSpot->edict();
}

void CBasePlayer::ResetStamina(void)
{
	pev->fuser2 = 0;
}

void CBasePlayer::Spawn(void)
{
	//ckf
	m_iDmgDone_Recent = 0;
	m_iGaitSequence = 0;
	m_flGaitFrame = 0;
	m_flGaitYaw = 0;
	m_vecPrevGaitOrigin = Vector(0, 0, 0);
	m_flGaitMovement = 0;

	m_bIsRespawning = false;
	m_bAllowAttack = true;
	//清除Buff
	Condition_Clear();
	//死亡方式
	m_bCritKilled = FALSE;
	m_bBackStabKilled = FALSE;
	m_bHeadShotKilled = FALSE;
	//恢复隐身
	m_iCloak = CLOAK_NO;
	pev->renderamt = 255;
	pev->rendermode = kRenderNormal;
	//解除伪装
	Disguise_Stop();
	//清理Ubercharge
	m_iUbercharge = 0;
	//清理抛射物
	ClearSticky();
	DmgRecord_Clear();

	//close HUDMenu here
	ShowHudMenu(this, MENU_CLOSE, 0, FALSE);
	m_iHudMenu = 0;

	if (pev->classname)
		RemoveEntityHashValue(pev, STRING(pev->classname), CLASSNAME);

	pev->classname = MAKE_STRING("player");
	AddEntityHashValue(pev, STRING(pev->classname), CLASSNAME);

	//清理HUD
	pev->armorvalue = 0;
	pev->armortype = 0;
	//pev->iuser4 = 1;

	//skin
	pev->skin = (m_iTeam == 1) ? 0 : 1;

	//become my desired class!
	m_iClass = m_iNewClass;

	m_iRocketJump = 0;
	m_iMultiJumpCurrent = 0;
	m_iMultiJumpMax = 0;
	m_bMultiJump = 0;

	m_pHealer = NULL;
	m_iHealerCount = 0;

	m_iCarryBluePrint = 0;
	m_pCarryBuild = NULL;

	pev->max_health = pev->health = GetClassMaxHealth(m_iClass);

	//tell the client we are spawned
	MESSAGE_BEGIN(MSG_ONE, gmsgSpawnInit, NULL, pev);
	WRITE_SHORT(pev->max_health);
	WRITE_BYTE(m_iClass);
	MESSAGE_END();

	switch(m_iClass)
	{
	default:
	case CLASS_SCOUT:
		pev->view_ofs = VEC_VIEW_SCOUT;
		m_iMultiJumpMax=1;
		break;
	case CLASS_HEAVY:
		pev->view_ofs = VEC_VIEW_HEAVY;
		break;
	case CLASS_SOLDIER:
		pev->view_ofs = VEC_VIEW_SOLDIER;
		break;
	case CLASS_PYRO:
		pev->view_ofs = VEC_VIEW_PYRO;
		break;
	case CLASS_SNIPER:
		pev->view_ofs = VEC_VIEW_SNIPER;
		break;
	case CLASS_MEDIC:
		pev->view_ofs = VEC_VIEW_MEDIC;
		m_fUbercharge = 0;
		break;
	case CLASS_ENGINEER:
		pev->view_ofs = VEC_VIEW_ENGINEER;
		m_iMetal = 200;
		break;
	case CLASS_DEMOMAN:
		pev->view_ofs = VEC_VIEW_DEMOMAN;
		break;
	case CLASS_SPY:
		pev->view_ofs = VEC_VIEW_SPY;
		m_flCloakEnergy = 100;
		break;
	}

	//is it time to clear all buildings?
	if(m_bShouldClearBuild)
	{
		Build_DestroyAll();
		m_bShouldClearBuild = false;
	}
	else if(m_iClass != CLASS_ENGINEER)//or are we no longer an engineer?
	{
		Build_DestroyAll();
	}

	pev->fuser4 = pev->view_ofs.z;

	pev->maxspeed = 1000;
	pev->takedamage = DAMAGE_AIM;
	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_WALK;
	pev->flags &= FL_PROXY;
	pev->flags |= FL_CLIENT;
	pev->air_finished = gpGlobals->time + 12;
	pev->dmg = 2;
	pev->effects = 0;
	pev->deadflag = DEAD_NO;
	pev->dmg_take = 0;
	pev->dmg_save = 0;

	m_bitsHUDDamage = -1;
	m_bitsDamageType = 0;
	m_afPhysicsFlags = 0;
	m_iClientFOV = -1;
	m_iClientHideHUD = -1;
	m_pentControlPoint = NULL;

	ResetStamina();
	pev->friction = 1;
	pev->gravity = 1;

	g_engfuncs.pfnSetPhysicsKeyValue(edict(), "hl", "1");
	m_hintMessageQueue.Reset();

	m_flVelocityModifier = 1;
	m_flLastTalk = 0;
	m_flIdleCheckTime = 0;
	m_flNextRadioTime = 0;
	m_iRadioLeft = 60;
	m_bInjuredBlast = FALSE;
	m_bInjuredExplosion = FALSE;
	m_flDisplayHistory &= ~(DHF_ROUND_STARTED | DHF_HOSTAGE_KILLED | DHF_TEAMMATE_WARRING | DHF_BOMBGUY);
	m_flHandleSignalDelay = 0;
	m_flUpdateSignalTime = gpGlobals->time;
	m_iVotingKikedPlayerId = 0;
	m_flNextVoteTime = 0;
	m_bJustKilledTeammate = FALSE;

	SET_VIEW(ENT(pev), ENT(pev));

	m_hObserverTarget = NULL;
	pev->iuser1 = pev->iuser2 = pev->iuser3 = 0;
	m_flLastAttackTime = -15;

	m_canSwitchObserverModes = true;

	m_iNumSpawns++;

	pev->fov = m_iFOV = m_iDefaultFOV;//Reset Fov
	m_flNextDecalTime = 0;
	m_flTimeStepSound = 0;
	m_iStepLeft = 0;
	m_flFieldOfView = 0.5;
	m_bloodColor = BLOOD_COLOR_RED;
	m_flgeigerDelay = gpGlobals->time + 2;
	m_flNextAttack = UTIL_WeaponTimeBase();

	StartSneaking();

	pev->body = 0;//SetBody

	/*if (m_bMissionBriefing == TRUE)
	{
		RemoveLevelText();
		m_bMissionBriefing = FALSE;
	}*/

	m_flFallVelocity = 0;
	g_pGameRules->GetPlayerSpawnSpot(this);
	//SET_MODEL(ENT(pev), "models/player.mdl");
	pev->sequence = LookupActivity(ACT_IDLE);

	SetPlayerModel();

	if (pev->flags & FL_DUCKING)
		UTIL_SetSize(pev, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);
	else
		UTIL_SetSize(pev, VEC_HULL_MIN, VEC_HULL_MAX);

	Precache();
	m_HackedGunPos = Vector(0, 32, 0);

	if (m_iPlayerSound == SOUNDLIST_EMPTY)
		ALERT(at_console, "Couldn't alloc player sound slot!\n");

	m_iHideHUD &= ~(HIDEHUD_WEAPONS | HIDEHUD_HEALTH | HIDEHUD_TIMER | HIDEHUD_MONEY);
	//m_iHideHUD = (HIDEHUD_WEAPONS | HIDEHUD_HEALTH | HIDEHUD_TIMER | HIDEHUD_MONEY);
	m_fNoPlayerSound = FALSE;
	m_pLastItem = NULL;
	m_fWeapon = FALSE;
	m_pClientActiveItem = NULL;
	m_iClientBattery = -1;
	m_iClientCharge = -1;
	m_fInitHUD = TRUE;

	if (!m_bNotKilled)
	{
		m_iClientHideHUD = -1;

		for (int i = 0; i < MAX_AMMO_SLOTS; i++)
			m_rgAmmo[i] = 0;

		m_bHasPrimaryWeapon = FALSE;
		m_bHasNightVision = FALSE;
	}
	else
	{
		for (int i = 0; i < MAX_AMMO_SLOTS; i++)
			m_rgAmmoLast[i] = -1;
	}

	m_lastx = m_lasty = 0;
	m_bNotKilled = TRUE;
	g_pGameRules->PlayerSpawn(this);

	ResetMaxSpeed();
	UTIL_SetOrigin(pev, pev->origin);

	SetScoreboardAttributes();

	MESSAGE_BEGIN(MSG_ALL, gmsgTeamInfo);
	WRITE_BYTE(ENTINDEX(edict()));
	WRITE_STRING(GetTeam(m_iTeam));
	MESSAGE_END();

	SendScoreInfo();

	SendRespawnTime(this, 0xFF);

	if (m_bNameChanged)
	{
		char *infobuffer = g_engfuncs.pfnGetInfoKeyBuffer(ENT(pev));
		char *curname = g_engfuncs.pfnInfoKeyValue(infobuffer, "name");

		if (strcmp(m_szNewName, curname))
			g_engfuncs.pfnSetClientKeyValue(entindex(), infobuffer, "name", m_szNewName);

		m_bNameChanged = FALSE;
		m_szNewName[0] = '\0';
	}

	UTIL_ScreenFade(this, Vector(0, 0, 0), 0.001, 0, 0, 0);
	SyncRoundTimer();
}

void CBasePlayer::Precache(void)
{
	if (WorldGraph.m_fGraphPresent && !WorldGraph.m_fGraphPointersSet)
	{
		if (!WorldGraph.FSetGraphPointers())
			ALERT(at_console, "**Graph pointers were not set!\n");
		else
			ALERT(at_console, "**Graph Pointers Set!\n");
	}

	m_flgeigerRange = 1000;
	m_igeigerRangePrev = 1000;
	m_bitsDamageType = 0;
	m_bitsHUDDamage = -1;
	m_iClientBattery = -1;
	m_iClientCharge = -1;
	m_iTrain = TRAIN_NEW;

	LinkUserMessages();
	m_iUpdateTime = 5;

	if (gInitHUD)
		m_fInitHUD = TRUE;
}

int CBasePlayer::Save(CSave &save)
{
	if (!CBaseMonster::Save(save))
		return 0;

	return save.WriteFields("PLAYER", this, m_playerSaveData, ARRAYSIZE(m_playerSaveData));
}

void CBasePlayer::SetScoreboardAttributes(CBasePlayer *pPlayer)
{
	if (pPlayer)
	{
		int flags = 0;

		if (pev->deadflag != DEAD_NO)
			flags |= SCOREATTRIB_DEAD;
		
		//flags |= SCOREATTRIB_VIP;//test

		if (gmsgScoreAttrib)
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgScoreAttrib, NULL, pPlayer->pev);
			WRITE_BYTE(ENTINDEX(edict()));
			WRITE_BYTE(flags);
			WRITE_BYTE(m_iClass);
			MESSAGE_END();
		}
	}
	else
	{
		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBasePlayer *pOther = (CBasePlayer *)UTIL_PlayerByIndex(i);

			if (pOther && !FNullEnt(pOther->edict()))
				SetScoreboardAttributes(pOther);
		}
	}
}

void CBasePlayer::RenewItems(void)
{
}

int CBasePlayer::Restore(CRestore &restore)
{
	if (!CBaseMonster::Restore(restore))
		return 0;

	int status = restore.ReadFields("PLAYER", this, m_playerSaveData, ARRAYSIZE(m_playerSaveData));
	SAVERESTOREDATA *pSaveData = (SAVERESTOREDATA *)gpGlobals->pSaveData;

	if (!pSaveData->fUseLandmark)
	{
		ALERT(at_console, "No Landmark:%s\n", pSaveData->szLandmarkName);

		edict_t *pentSpawnSpot = EntSelectSpawnPoint(this);
		pev->origin = VARS(pentSpawnSpot)->origin + Vector(0, 0, 1);
		pev->angles = VARS(pentSpawnSpot)->angles;
	}

	pev->angles.z = 0;
	pev->angles = pev->v_angle;
	pev->fixangle = TRUE;
	m_bloodColor = BLOOD_COLOR_RED;
	m_ulModelIndexPlayer = pev->modelindex;

	if (pev->flags & FL_DUCKING)
		UTIL_SetSize(pev, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX);
	else
		UTIL_SetSize(pev, VEC_HULL_MIN, VEC_HULL_MAX);

	m_flDisplayHistory &= ~(DHF_HOSTAGE_SEE | DHF_HOSTAGE_FOLLOW | DHF_HOSTAGE_FOLLOWED | DHF_HOSTAGE_WARRING | DHF_SPOTTED_FRIEND | DHF_SPOTTED_ENEMY | DHF_TEAMMATE_INJURE | DHF_KILLEDENEMY | DHF_AMMO_NOTENOUGH | DHF_ENTERZONE_BOMB | DHF_ENTERZONE_RESCUE | DHF_ENTERZONE_ESCAPE | DHF_ENTERZONE_VIPSAFETY | DHF_HOSTAGE_CANTMOVE);
	SetScoreboardAttributes();
	return status;
}

void CBasePlayer::Restart(void)
{
	m_bNotKilled = FALSE;

	pev->frags = 0;
	m_iDominates = 0;
	memset(&m_Dominate, 0, sizeof(m_Dominate));

	SendScoreInfo();

	Build_DestroyAll();
}

void CBasePlayer::SelectNextItem(int iItem)
{
	CBasePlayerItem *pItem = m_rgpPlayerItems[iItem];

	if (!pItem)
		return;

	if (pItem == m_pActiveItem)
	{
		pItem = m_pActiveItem->m_pNext;

		if (!pItem)
			return;

		CBasePlayerItem *pLast = pItem;

		while (pLast->m_pNext)
			pLast = pLast->m_pNext;

		pLast->m_pNext = m_pActiveItem;
		m_pActiveItem->m_pNext = NULL;
		m_rgpPlayerItems[iItem] = pItem;
	}

	ResetAutoaim();

	if (m_pActiveItem)
		m_pActiveItem->Holster();

	m_pActiveItem = pItem;

	if (m_pActiveItem)
	{
		m_pActiveItem->Deploy();
		m_pActiveItem->UpdateItemInfo();
		ResetMaxSpeed();
	}
}

void CBasePlayer::SelectItem(const char *pstr)
{
	if (!pstr)
		return;

	CBasePlayerItem *pItem = NULL;

	for (int i = 0; i < MAX_ITEM_TYPES; i++)
	{
		if (m_rgpPlayerItems[i])
		{
			pItem = m_rgpPlayerItems[i];

			while (pItem)
			{
				if (FClassnameIs(pItem->pev, pstr))
					break;

				pItem = pItem->m_pNext;
			}
		}

		if (pItem)
			break;
	}

	if (!pItem)
		return;

	if (pItem == m_pActiveItem)
		return;

	ResetAutoaim();

	if (m_pActiveItem)
		m_pActiveItem->Holster();

	m_pLastItem = m_pActiveItem;
	m_pActiveItem = pItem;

	if (m_pLastItem && m_pActiveItem->CanDeploy())
	{
		m_pActiveItem->Deploy();
		m_pActiveItem->UpdateItemInfo();
		ResetMaxSpeed();
	}
}

void CBasePlayer::SelectLastItem(void)
{
	if (!m_pLastItem)
		return;

	if (m_pActiveItem && !m_pActiveItem->CanHolster())
		return;

	ResetAutoaim();

	if (m_pActiveItem)
		m_pActiveItem->Holster();

	CBasePlayerItem *pTemp = m_pActiveItem;
	m_pActiveItem = m_pLastItem;
	m_pLastItem = pTemp;
	m_pActiveItem->Deploy();
	m_pActiveItem->UpdateItemInfo();
	ResetMaxSpeed();
}

BOOL CBasePlayer::HasWeapons(void)
{
	for (int i = 0; i < MAX_ITEM_TYPES; i++)
	{
		if (m_rgpPlayerItems[i])
			return TRUE;
	}

	return FALSE;
}

void CBasePlayer::SelectPrevItem(int iItem)
{
}

const char *CBasePlayer::TeamID(void)
{
	if (!pev)
		return "";

	return GetTeam(m_iTeam);
}

class CSprayCan : public CBaseEntity
{
public:
	void Spawn(entvars_t *pevOwner);
	void Think(void);
	int ObjectCaps(void) { return FCAP_DONT_SAVE; }
};

void CSprayCan::Spawn(entvars_t *pevOwner)
{
	pev->origin = pevOwner->origin + Vector(0, 0, 32);
	pev->angles = pevOwner->v_angle;
	pev->owner = ENT(pevOwner);
	pev->frame = 0;
	pev->nextthink = gpGlobals->time + 0.1;

	EMIT_SOUND(ENT(pev), CHAN_VOICE, "player/sprayer.wav", VOL_NORM, ATTN_NORM);
}

void CSprayCan::Think(void)
{
	TraceResult tr;
	CBasePlayer *pPlayer = (CBasePlayer *)GET_PRIVATE(pev->owner);
	int nFrames;

	if (pPlayer)
		nFrames = pPlayer->GetCustomDecalFrames();
	else
		nFrames = -1;

	int playernum = ENTINDEX(pev->owner);

	UTIL_MakeVectors(pev->angles);
	UTIL_TraceLine(pev->origin, pev->origin + gpGlobals->v_forward * 128, ignore_monsters, pev->owner, &tr);

	if (nFrames == -1)
	{
		UTIL_DecalTrace(&tr, DECAL_LAMBDA6);
		UTIL_Remove(this);
	}
	else
	{
		UTIL_PlayerDecalTrace(&tr, playernum, pev->frame, TRUE);

		if (pev->frame++ >= (nFrames - 1))
			UTIL_Remove(this);
	}

	pev->nextthink = gpGlobals->time + 0.1;
}

class CBloodSplat : public CBaseEntity
{
public:
	void Spawn(entvars_t *pevOwner);
	void Spray(void);
};

void CBloodSplat::Spawn(entvars_t *pevOwner)
{
	pev->origin = pevOwner->origin + Vector(0, 0, 32);
	pev->angles = pevOwner->v_angle;
	pev->owner = ENT(pevOwner);

	SetThink(&CBloodSplat::Spray);
	pev->nextthink = gpGlobals->time + 0.1;
}

void CBloodSplat::Spray(void)
{
	if (g_Language != LANGUAGE_GERMAN)
	{
		TraceResult tr;
		UTIL_MakeVectors(pev->angles);
		UTIL_TraceLine(pev->origin, pev->origin + gpGlobals->v_forward * 128, ignore_monsters, pev->owner, &tr);
		UTIL_BloodDecalTrace(&tr, BLOOD_COLOR_RED);
	}

	SetThink(&CBaseEntity::SUB_Remove);
	pev->nextthink = gpGlobals->time + 0.1;
}

void CBasePlayer::GiveNamedItem(const char *pszName)
{
	edict_t *pent = CREATE_NAMED_ENTITY(MAKE_STRING(pszName));

	if (FNullEnt(pent))
	{
		ALERT(at_console, "NULL Ent in GiveNamedItem!");
		return;
	}

	pent->v.origin = pev->origin;
	pent->v.spawnflags |= SF_NORESPAWN;

	DispatchSpawn(pent);
	DispatchTouch(pent, ENT(pev));
}

CBaseEntity *FindEntityForward(CBaseEntity *pEntity)
{
	UTIL_MakeVectors(pEntity->pev->v_angle);

	TraceResult tr;
	Vector vecSrc = pEntity->pev->origin + pEntity->pev->view_ofs;
	Vector vecEnd = vecSrc + gpGlobals->v_forward * 8192;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, pEntity->edict(), &tr);

	if (tr.flFraction != 1 && !FNullEnt(tr.pHit))
		return CBaseEntity::Instance(tr.pHit);

	return NULL;
}

void CBasePlayer::ForceClientDllUpdate(void)
{
	m_fInitHUD = TRUE;
	m_iClientHealth = -1;
	m_iClientBattery = -1;
	m_iClientCharge = -1;
	m_iTrain |= TRAIN_NEW;
	m_fWeapon = FALSE;
	m_fKnownItem = FALSE;

	UpdateClientData();
}

int CBasePlayer::FShouldCollide(CBaseEntity *pHit)
{
	if(pHit->IsBSPModel() && pHit->pev->team != 0)
	{
		if(pHit->pev->team == m_iTeam)
			return 0;
		if(pHit->pev->team == 3)
			return 0;
		if(g_pGameRules->m_iRoundStatus == ROUND_END)
			return 0;
		return 1;
	}
	if(!g_fIsTraceLine)
	{
		if(pHit->IsPlayer())
		{
			CBasePlayer *pPlayer = (CBasePlayer *)pHit;
			if(pPlayer->m_iTeam == m_iTeam && pev != pPlayer->pev)
				return 0;
			return 1;
		}
		else if(pHit->Classify() == CLASS_BUILDABLE)
		{
			CBaseBuildable *pBuild = (CBaseBuildable *)pHit;
			if(!pBuild->m_pPlayer)
				return 0;
			if(pBuild->GetBuildClass() == BUILDABLE_ENTRANCE || pBuild->GetBuildClass() == BUILDABLE_EXIT)
				return 1;
			if(pBuild->m_iTeam == m_iTeam && pBuild->m_pPlayer->pev != pev)
				return 0;
			return 1;
		}
		else if(pHit->Classify() == CLASS_PROJECTILE)//just walk through any projectiles
		{
			return 0;
		}
	}
	return 1;
}

extern float g_flWeaponCheat;

void CBasePlayer::ImpulseCommands(void)
{
	PlayerUse();

	TraceResult tr;
	int iImpulse = pev->impulse;

	switch (iImpulse)
	{
		case 99:
		{
			int iOn;

			if (!gmsgLogo)
			{
				iOn = 1;
				gmsgLogo = REG_USER_MSG("Logo", 1);
			}
			else
				iOn = 0;

			MESSAGE_BEGIN(MSG_ONE, gmsgLogo, NULL, pev);
			WRITE_BYTE(iOn);
			MESSAGE_END();

			if (!iOn)
				gmsgLogo = 0;

			break;
		}

		case 201:
		{
			if (gpGlobals->time < m_flNextDecalTime)
				break;

			UTIL_MakeVectors(pev->v_angle);
			UTIL_TraceLine(pev->origin + pev->view_ofs, pev->origin + pev->view_ofs + gpGlobals->v_forward * 128, ignore_monsters, edict(), &tr);

			if (tr.flFraction != 1)
			{
				m_flNextDecalTime = gpGlobals->time + CVAR_GET_FLOAT("decalfrequency");
				CSprayCan *pCan = GetClassPtr((CSprayCan *)NULL);
				pCan->Spawn(pev);
			}

			break;
		}

		default: CheatImpulseCommands(iImpulse);
	}

	pev->impulse = 0;
}

void CBasePlayer::CheatImpulseCommands(int iImpulse)
{
	TraceResult tr;

	if (!g_flWeaponCheat)
		return;

	switch (iImpulse)
	{
		case 76:
		{
			if (giPrecacheGrunt)
			{
				UTIL_MakeVectors(Vector(0, pev->v_angle.y, 0));
				Create("monster_human_grunt", pev->origin + gpGlobals->v_forward * 128, pev->angles);
			}
			else
			{
				giPrecacheGrunt = 1;
				ALERT(at_console, "You must now restart to use Grunt-o-matic.\n");
			}

			break;
		}

		case 101:
		{
			gEvilImpulse101 = TRUE;
			//AddAccount(16000, TRUE);
			//ALERT(at_console, "Crediting %s with $16000\n", STRING(pev->netname));
			break;
		}

		case 102: CGib::SpawnRandomGibs(pev, 1, 1); break;

		case 103:
		{
			CBaseEntity *pEntity = FindEntityForward(this);

			if (pEntity)
			{
				CBaseMonster *pMonster = pEntity->MyMonsterPointer();

				if (pMonster)
					pMonster->ReportAIState();
			}

			break;
		}

		case 104: gGlobalState.DumpGlobals(); break;

		case 105:
		{
			if (m_fNoPlayerSound)
			{
				ALERT(at_console, "Player is audible\n");
				m_fNoPlayerSound = FALSE;
			}
			else
			{
				ALERT(at_console, "Player is silent\n");
				m_fNoPlayerSound = TRUE;
			}

			break;
		}

		case 106:
		{
			CBaseEntity *pEntity = FindEntityForward(this);

			if (pEntity)
			{
				ALERT(at_console, "Classname: %s", STRING(pEntity->pev->classname));

				if (!FStringNull(pEntity->pev->targetname))
					ALERT(at_console, " - Targetname: %s\n", STRING(pEntity->pev->targetname));
				else
					ALERT(at_console, " - TargetName: No Targetname\n");

				ALERT(at_console, "Model: %s\n", STRING(pEntity->pev->model));

				if (pEntity->pev->globalname)
					ALERT(at_console, "Globalname: %s\n", STRING(pEntity->pev->globalname));
			}

			break;
		}

		case 107:
		{
			edict_t *pWorld = g_engfuncs.pfnPEntityOfEntIndex(0);
			Vector start = pev->origin + pev->view_ofs;
			Vector end = start + gpGlobals->v_forward * 1024;
			UTIL_TraceLine(start, end, ignore_monsters, edict(), &tr);

			if (tr.pHit)
				pWorld = tr.pHit;

			const char *pszTextureName = TRACE_TEXTURE(pWorld, start, end);

			if (pszTextureName)
				ALERT(at_console, "Texture: %s\n", pszTextureName);

			break;
		}

		case 195: Create("node_viewer_fly", pev->origin, pev->angles); break;
		case 196: Create("node_viewer_large", pev->origin, pev->angles); break;
		case 197: Create("node_viewer_human", pev->origin, pev->angles); break;

		case 199:
		{
			ALERT(at_console, "%d\n", WorldGraph.FindNearestNode(pev->origin, bits_NODE_GROUP_REALM));
			WorldGraph.ShowNodeConnections(WorldGraph.FindNearestNode(pev->origin, bits_NODE_GROUP_REALM));
			break;
		}

		case 202:
		{
			UTIL_MakeVectors(pev->v_angle);
			UTIL_TraceLine(pev->origin + pev->view_ofs, pev->origin + pev->view_ofs + gpGlobals->v_forward * 128, ignore_monsters, edict(), &tr);

			if (tr.flFraction != 1)
			{
				CBloodSplat *pBlood = GetClassPtr((CBloodSplat *)NULL);
				pBlood->Spawn(pev);
			}

			break;
		}

		case 203:
		{
			CBaseEntity *pEntity = FindEntityForward(this);

			if (pEntity && pEntity->pev->takedamage != DAMAGE_NO)
				pEntity->SetThink(&CBaseEntity::SUB_Remove);

			break;
		}
	}
}

void CBasePlayer::HandleSignals(void)
{
	if (g_pGameRules->IsMultiplayer())
	{
		if (!g_pGameRules->m_bMapHasResupplyRoom)
		{
			if (m_iTeam == TEAM_TERRORIST || m_iTeam == TEAM_CT)
			{
				CBaseEntity *pEntity = NULL;
				char *classname = (m_iTeam == TEAM_TERRORIST) ? "info_player_deathmatch" : "info_player_start";

				while ((pEntity = UTIL_FindEntityByClassname(pEntity, classname)) != NULL)
				{
					if ((pEntity->pev->origin - pev->origin).Length() < 200)
						m_iMapZone |= MAPZONE_RESUPPLYROOM;
				}
			}
		}
	}
	int zoneSave = m_iMapZone;
	int zoneChanged = m_iClientMapZone ^ m_iMapZone;
	m_iClientMapZone = m_iMapZone;
	m_iMapZone = 0;

	if(zoneChanged & MAPZONE_CONTROLPOINT)
	{
		//Entering control point
		if(zoneSave & MAPZONE_CONTROLPOINT)
		{

		}
		else
		{//Leaving control point

		}
	}
}

int CBasePlayer::AddPlayerItem(CBasePlayerItem *pItem)
{
	CBasePlayerItem *pInsert = m_rgpPlayerItems[pItem->iItemSlot()];

	while (pInsert)
	{
		if (FClassnameIs(pInsert->pev, STRING(pItem->pev->classname)))
		{
			if (pItem->AddDuplicate(pInsert))
			{
				g_pGameRules->PlayerGotWeapon(this, pItem);
				pItem->CheckRespawn();
				pItem->UpdateItemInfo();

				if (m_pActiveItem)
					m_pActiveItem->UpdateItemInfo();

				pItem->Kill();
			}
			else if (gEvilImpulse101)
				pItem->Kill();

			return FALSE;
		}

		pInsert = pInsert->m_pNext;
	}

	if (pItem->AddToPlayer(this))
	{
		g_pGameRules->PlayerGotWeapon(this, pItem);

		if (pItem->iItemSlot() == WEAPON_SLOT_PRIMARY)
			m_bHasPrimaryWeapon = TRUE;

		pItem->CheckRespawn();
		pItem->m_pNext = m_rgpPlayerItems[pItem->iItemSlot()];
		m_rgpPlayerItems[pItem->iItemSlot()] = pItem;

		SendWeaponUpdate(pItem);

		if (g_pGameRules->FShouldSwitchWeapon(this, pItem))
			SwitchWeapon(pItem);

		return TRUE;
	}
	else if (gEvilImpulse101)
		pItem->Kill();

	return FALSE;
}

int CBasePlayer::RemovePlayerItem(CBasePlayerItem *pItem)
{
	if (m_pActiveItem == pItem)
	{
		ResetAutoaim();
		pItem->pev->nextthink = 0;
		pItem->SetThink(NULL);
		m_pActiveItem = NULL;
		pev->viewmodel = 0;
		pev->weaponmodel = 0;
	}
	else if (m_pLastItem == pItem)
		m_pLastItem = NULL;

	CBasePlayerItem *pPrev = m_rgpPlayerItems[pItem->iItemSlot()];

	if (pPrev == pItem)
	{
		m_rgpPlayerItems[pItem->iItemSlot()] = pItem->m_pNext;
		return TRUE;
	}

	while (pPrev && pPrev->m_pNext != pItem)
		pPrev = pPrev->m_pNext;

	if (pPrev)
	{
		pPrev->m_pNext = pItem->m_pNext;
		return TRUE;
	}

	return FALSE;
}

int CBasePlayer::GiveAmmo(int iCount, char *szName, int iMax)
{
	if (pev->flags & FL_SPECTATOR)
		return -1;

	if (!szName)
		return -1;

	if (!g_pGameRules->CanHaveAmmo(this, szName, iMax))
		return -1;

	int i = GetAmmoIndex(szName);

	if (i < 0 || i >= MAX_AMMO_SLOTS)
		return -1;

	int iAdd = min(iCount, iMax - m_rgAmmo[i]);

	if (iAdd < 1)
		return i;

	m_rgAmmo[i] += iAdd;

	if (gmsgAmmoPickup)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgAmmoPickup, NULL, pev);
		WRITE_BYTE(GetAmmoIndex(szName));
		WRITE_BYTE(iAdd);
		MESSAGE_END();
	}

	return i;
}

void CBasePlayer::ItemPreFrame(void)
{
#ifdef CLIENT_WEAPONS
	if (m_flNextAttack > 0)
		return;
#else
	if (gpGlobals->time < m_flNextAttack)
		return;
#endif

	if (m_pActiveItem)
		m_pActiveItem->ItemPreFrame();
}

void CBasePlayer::ItemPostFrame(void)
{
	if (m_pTank != 0)
		return;

#ifdef CLIENT_WEAPONS
	if (m_flNextAttack > 0)
		return;
#else
	if (gpGlobals->time < m_flNextAttack)
		return;
#endif

	ImpulseCommands();

	if (m_pActiveItem)
		m_pActiveItem->ItemPostFrame();
}

int CBasePlayer::AmmoInventory(int iAmmoIndex)
{
	if (iAmmoIndex == -1)
		return -1;

	return m_rgAmmo[iAmmoIndex];
}

int CBasePlayer::GetAmmoIndex(const char *psz)
{
	if (!psz)
		return -1;

	for (int i = 1; i < MAX_AMMO_SLOTS; i++)
	{
		if (!CBasePlayerItem::AmmoInfoArray[i].pszName)
			continue;

		if (!stricmp(psz, CBasePlayerItem::AmmoInfoArray[i].pszName))
			return i;
	}

	return -1;
}

void CBasePlayer::SendWeaponUpdate(CBasePlayerItem *pItem)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgWeaponInfo, NULL, pev);
	WRITE_BYTE(pItem->m_iId);
	WRITE_BYTE(pItem->iItemSlot());
	WRITE_BYTE(pItem->iMaxClip() < 0 ? 0 : pItem->iMaxClip());
	WRITE_BYTE(pItem->iMaxAmmo1() < 0 ? 0 : pItem->iMaxAmmo1());
	WRITE_BYTE(pItem->iBody());
	WRITE_BYTE(pItem->iSkin());
	MESSAGE_END();
}

void CBasePlayer::SendAmmoUpdate(void)
{
	if(m_pActiveItem)
	{
		if(m_pActiveItem->IsWeapon())
		{
			CBasePlayerWeapon *pWeapon = (CBasePlayerWeapon *)m_pActiveItem;
			if(pWeapon->m_iPrimaryAmmoType >= 0)
			{
				MESSAGE_BEGIN(MSG_ONE, gmsgAmmoX, NULL, pev);
				WRITE_BYTE(pWeapon->m_iId);
				WRITE_BYTE(max(min(m_rgAmmo[pWeapon->m_iPrimaryAmmoType], 255), 0));
				MESSAGE_END();
			}
		}
	}
}

void CBasePlayer::SendHitDamage(entvars_t *pevVictim, int iDamage, int iCrit)
{
	if(m_flHitDamageTimer > gpGlobals->time)
		return;

	m_flHitDamageTimer = gpGlobals->time + 0.1;

	MESSAGE_BEGIN(MSG_ONE, gmsgDrawFX, NULL, pev);
	WRITE_BYTE(FX_HITDAMAGE);
	WRITE_SHORT(ENTINDEX(ENT(pevVictim)));
	WRITE_SHORT(iDamage);
	WRITE_BYTE(iCrit);
	MESSAGE_END();
}

void CBasePlayer::SendAddHealth(int iHeal)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgDrawFX, NULL, pev);
	WRITE_BYTE(FX_ADDHEALTH);
	WRITE_SHORT(iHeal);
	MESSAGE_END();
}

void CBasePlayer::SendAddTime(int iTime)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgDrawFX, NULL, pev);
	WRITE_BYTE(FX_ADDTIME);
	WRITE_SHORT(iTime);
	MESSAGE_END();
}

void CBasePlayer::SendAddMetal(int iMetal)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgDrawFX, NULL, pev);
	WRITE_BYTE(FX_ADDMETAL);
	WRITE_SHORT(iMetal);
	MESSAGE_END();
}

void CBasePlayer::SendCriticalHit(BOOL bShowEffects, BOOL bShowSound)
{
	if(m_flCritHitTimer > gpGlobals->time)
		return;

	m_flCritHitTimer = gpGlobals->time + 0.1;

	MESSAGE_BEGIN(MSG_PVS, gmsgDrawFX, pev->origin);
	WRITE_BYTE(FX_CRITHIT);
	WRITE_SHORT(entindex());
	WRITE_BYTE((bShowEffects & 1) | ((bShowSound & 1) << 1));
	MESSAGE_END();
}

void CBasePlayer::SendMiniCritHit(BOOL bShowEffects, BOOL bShowSound)
{
	if(m_flCritHitTimer > gpGlobals->time)
		return;

	m_flCritHitTimer = gpGlobals->time + 0.1;

	MESSAGE_BEGIN(MSG_PVS, gmsgDrawFX, pev->origin);
	WRITE_BYTE(FX_MINICRITHIT);
	WRITE_SHORT(entindex());
	WRITE_BYTE((bShowEffects & 1) | ((bShowSound & 1) << 1));
	MESSAGE_END();
}

void CBasePlayer::UpdateClientData(void)
{
	if (m_fInitHUD)
	{
		m_fInitHUD = FALSE;
		gInitHUD = FALSE;
		m_iClientMapZone = m_iMapZone;
		m_iMapZone = 0;

		MESSAGE_BEGIN(MSG_ONE, gmsgResetHUD, NULL, pev);
		MESSAGE_END();

		if (!m_fGameHUDInitialized)
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgInitHUD, NULL, pev);
			MESSAGE_END();

			g_pGameRules->InitHUD(this);
			m_fGameHUDInitialized = TRUE;

			if (g_pGameRules->IsMultiplayer())
				FireTargets("game_playerjoin", this, this, USE_TOGGLE, 0);

			m_iObserverLastMode = OBS_CHASE_FREE;
			m_iObserverC4State = 0;
			m_bObserverHasDefuser = false;
			SetObserverAutoDirector(false);
		}

		FireTargets("game_playerspawn", this, this, USE_TOGGLE, 0);

		SyncRoundTimer();
		g_pGameRules->CPSendInit(this);
		g_pGameRules->CPSendState(this);
		g_pGameRules->RTSendInit(this);
		g_pGameRules->RTSendState(this);

		if (g_pGameRules->IsMultiplayer())
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgTeamScore, NULL, pev);
			WRITE_STRING(GetTeam(TEAM_CT));
			WRITE_SHORT(g_pGameRules->m_iNumCTWins);
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_ONE, gmsgTeamScore, NULL, pev);
			WRITE_STRING(GetTeam(TEAM_TERRORIST));
			WRITE_SHORT(g_pGameRules->m_iNumTerroristWins);
			MESSAGE_END();
		}
	}

	if (m_iHideHUD != m_iClientHideHUD)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgHideWeapon, NULL, pev);
		WRITE_SHORT(m_iHideHUD);
		MESSAGE_END();

		m_iClientHideHUD = m_iHideHUD;
	}

	if (m_iFOV != m_iClientFOV)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgSetFOV, NULL, pev);
		WRITE_BYTE(m_iFOV);
		MESSAGE_END();

		MESSAGE_BEGIN(MSG_SPEC, gmsgHLTV);
		WRITE_BYTE(ENTINDEX(edict()));
		WRITE_BYTE(m_iFOV);
		MESSAGE_END();
	}

	if (gDisplayTitle)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgShowGameTitle, NULL, pev);
		WRITE_BYTE(0);
		MESSAGE_END();

		gDisplayTitle = FALSE;
	}

	if (pev->health != m_iClientHealth)
	{
		int iHelath = max(pev->health, 0);
		MESSAGE_BEGIN(MSG_ONE, gmsgHealth, NULL, pev);
		WRITE_SHORT(iHelath);
		MESSAGE_END();

		m_iClientHealth = (int)pev->health;
	}

	if (pev->armorvalue != m_iClientBattery)
	{
		m_iClientBattery = (int)pev->armorvalue;
		MESSAGE_BEGIN(MSG_ONE, gmsgBattery, NULL, pev);
		WRITE_SHORT((int)pev->armorvalue);
		MESSAGE_END();
	}

	if (pev->armortype != m_iClientCharge)
	{
		m_iClientCharge = (int)pev->armortype;
		MESSAGE_BEGIN(MSG_ONE, gmsgArmorType, NULL, pev);
		WRITE_BYTE((int)pev->armortype);
		MESSAGE_END();
	}

	if (pev->dmg_take || pev->dmg_save || m_bitsHUDDamage != m_bitsDamageType)
	{
		Vector damageOrigin = pev->origin;
		edict_t *other = pev->dmg_inflictor;

		if (other)
		{
			CBaseEntity *pEntity = CBaseEntity::Instance(other);

			if (pEntity)
				damageOrigin = pEntity->Center();
		}

		int visibleDamageBits = m_bitsDamageType & DMG_SHOWNHUD;

		MESSAGE_BEGIN(MSG_ONE, gmsgDamage, NULL, pev);
		WRITE_BYTE((int)pev->dmg_save);
		WRITE_BYTE((int)pev->dmg_take);
		WRITE_LONG(visibleDamageBits);
		WRITE_COORD(damageOrigin.x);
		WRITE_COORD(damageOrigin.y);
		WRITE_COORD(damageOrigin.z);
		MESSAGE_END();

		pev->dmg_take = 0;
		pev->dmg_save = 0;
		m_bitsHUDDamage = m_bitsDamageType;
		m_bitsDamageType &= DMG_TIMEBASED;
	}

	if (m_iTrain & TRAIN_NEW)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgTrain, NULL, pev);
		WRITE_BYTE(m_iTrain & 0xF);
		MESSAGE_END();

		m_iTrain &= ~TRAIN_NEW;
	}

	if (!m_fKnownItem)
	{
		m_fKnownItem = TRUE;

		for (int i = 0; i < MAX_WEAPONS; i++)
		{
			ItemInfo &II = CBasePlayerItem::ItemInfoArray[i];

			if (!II.iId)
				continue;

			const char *pszName;

			if (!II.pszName)
				pszName = "Empty";
			else
				pszName = II.pszName;

			MESSAGE_BEGIN(MSG_ONE, gmsgWeaponList, NULL, pev);
			WRITE_STRING(pszName);
			WRITE_BYTE(GetAmmoIndex(II.pszAmmo1));
			WRITE_SHORT(II.iMaxAmmo1);
			WRITE_BYTE(GetAmmoIndex(II.pszAmmo2));
			WRITE_SHORT(II.iMaxAmmo2);
			WRITE_BYTE(II.iSlot);
			WRITE_BYTE(II.iPosition);
			WRITE_BYTE(max(min(II.iId, MAX_WEAPONS), 0));//ckf
			WRITE_BYTE(II.iFlags);
			MESSAGE_END();

		}
	}

	SendAmmoUpdate();

	for (int i = 0; i < MAX_ITEM_TYPES; i++)
	{
		if (m_rgpPlayerItems[i])
			m_rgpPlayerItems[i]->UpdateClientData(this);
	}

	m_pClientActiveItem = m_pActiveItem;
	m_iClientFOV = m_iFOV;

	//We use client-side statusbar now
	/*if (m_flNextSBarUpdateTime < gpGlobals->time)
	{
		UpdateStatusBar();
		m_flNextSBarUpdateTime = gpGlobals->time + 0.2;
	}*/

	if (!(m_flDisplayHistory & DHF_AMMO_NOTENOUGH))
	{
		if (m_pActiveItem && m_pActiveItem->IsWeapon())
		{
			CBasePlayerWeapon *pWeapon = (CBasePlayerWeapon *)m_pActiveItem;

			if (!(pWeapon->iFlags() & ITEM_FLAG_EXHAUSTIBLE))
			{
				if (pWeapon->m_iPrimaryAmmoType != -1 && !m_rgAmmo[pWeapon->m_iPrimaryAmmoType] && !pWeapon->m_iClip)
				{
					m_flDisplayHistory |= DHF_AMMO_NOTENOUGH;
					HintMessage("#Hint_out_of_ammo");
				}
			}
		}
	}

	if (gpGlobals->time > m_flHandleSignalDelay)
	{
		m_flHandleSignalDelay = gpGlobals->time + 0.5;
		HandleSignals();
	}

	//Remember to send signals

	//Since we don't have a radar

/*	if (pev->deadflag == DEAD_NO)
	{
		if (gpGlobals->time > m_flUpdateSignalTime)
		{
			m_flUpdateSignalTime = gpGlobals->time + 1;

			
			//m_vecClientRadarPos = pev->origin;
		}
	}*/
}

BOOL CBasePlayer::FBecomeProne(void)
{
	m_afPhysicsFlags |= PFLAG_ONBARNACLE;
	return TRUE;
}

void CBasePlayer::BarnacleVictimBitten(entvars_t *pevBarnacle)
{
	TakeDamage(pevBarnacle, pevBarnacle, pev->health + pev->armorvalue, DMG_SLASH | DMG_ALWAYSGIB, 0);
}

void CBasePlayer::BarnacleVictimReleased(void)
{
	m_afPhysicsFlags &= ~PFLAG_ONBARNACLE;
}

int CBasePlayer::Illumination(void)
{
	int iIllum = CBaseEntity::Illumination() + m_iWeaponFlash;

	if (iIllum > 255)
		iIllum = 255;

	return iIllum;
}

void CBasePlayer::EnableControl(BOOL fControl)
{
	if (!fControl)
		pev->flags |= FL_FROZEN;
	else
		pev->flags &= ~FL_FROZEN;
}

void CBasePlayer::ResetMaxSpeed(void)
{
	float speed = GetPlayerMaxSpeed();

	if (IsObserver())
		speed = 1200;
	else if (g_pGameRules->m_bFreezePeriod)
		speed = 1;
	else if(m_iDisguise == DISGUISE_YES)
		speed = m_iDisguiseMaxSpeed;
	else if (m_pActiveItem)
		speed *= m_pActiveItem->GetMaxSpeed();

	g_engfuncs.pfnSetClientMaxspeed(ENT(pev), speed);
}

BOOL CBasePlayer::HintMessage(const char *message, BOOL bOnlyDead, BOOL bImportant)
{
	if (!bOnlyDead && !IsAlive())
		return FALSE;

	if (bImportant)
		return m_hintMessageQueue.AddMessage(message);

	if (m_bShowHints)
		return m_hintMessageQueue.AddMessage(message);

	return TRUE;
}

#define DOT_1DEGREE 0.9998476951564
#define DOT_2DEGREE 0.9993908270191
#define DOT_3DEGREE 0.9986295347546
#define DOT_4DEGREE 0.9975640502598
#define DOT_5DEGREE 0.9961946980917
#define DOT_6DEGREE 0.9945218953683
#define DOT_7DEGREE 0.9925461516413
#define DOT_8DEGREE 0.9902680687416
#define DOT_9DEGREE 0.9876883405951
#define DOT_10DEGREE 0.9848077530122
#define DOT_15DEGREE 0.9659258262891
#define DOT_20DEGREE 0.9396926207859
#define DOT_25DEGREE 0.9063077870367

Vector CBasePlayer::GetAutoaimVector(float flDelta)
{
	if (g_iSkillLevel == SKILL_HARD)
	{
		UTIL_MakeVectors(pev->v_angle + pev->punchangle);
		return gpGlobals->v_forward;
	}

	Vector vecSrc = GetGunPosition();
	m_vecAutoAim = Vector(0, 0, 0);
	BOOL m_fOldTargeting = m_fOnTarget;
	Vector angles = AutoaimDeflection(vecSrc, 8192, flDelta);

	if (!g_pGameRules->AllowAutoTargetCrosshair())
		m_fOnTarget = FALSE;
	else if (m_fOldTargeting != m_fOnTarget)
		m_pActiveItem->UpdateItemInfo();

	if (angles.x > 180)
		angles.x -= 360;

	if (angles.x < 180)
		angles.x += 360;

	if (angles.y > 180)
		angles.y -= 360;

	if (angles.y < 180)
		angles.y += 360;

	if (angles.x > 25)
		angles.x = 25;

	if (angles.x < -25)
		angles.x = -25;

	if (angles.y > 12)
		angles.y = 12;

	if (angles.y < -12)
		angles.y = -12;

	if (g_iSkillLevel == SKILL_EASY)
		m_vecAutoAim = m_vecAutoAim * 0.67 + angles * 0.33;
	else
		m_vecAutoAim = angles * 0.9;

	if (CVAR_GET_FLOAT("sv_aim"))
	{
		if (m_vecAutoAim.x != m_lastx || m_vecAutoAim.y != m_lasty)
		{
			SET_CROSSHAIRANGLE(ENT(pev), -m_vecAutoAim.x, m_vecAutoAim.y);

			m_lastx = m_vecAutoAim.x;
			m_lasty = m_vecAutoAim.y;
		}
	}

	UTIL_MakeVectors(pev->v_angle + pev->punchangle + m_vecAutoAim);
	return gpGlobals->v_forward;
}

Vector CBasePlayer::AutoaimDeflection(Vector &vecSrc, float flDist, float flDelta)
{
	edict_t *pEdict = g_engfuncs.pfnPEntityOfEntIndex(1);

	if (!CVAR_GET_FLOAT("sv_aim"))
	{
		m_fOnTarget = FALSE;
		return g_vecZero;
	}

	UTIL_MakeVectors(pev->v_angle + pev->punchangle + m_vecAutoAim);

	Vector bestdir = gpGlobals->v_forward;
	float bestdot = flDelta;
	edict_t *bestent = NULL;
	m_fOnTarget = FALSE;

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecSrc + bestdir * flDist, dont_ignore_monsters, edict(), &tr);

	if (tr.pHit && tr.pHit->v.takedamage != DAMAGE_NO)
	{
		if (!((pev->waterlevel != 3 && tr.pHit->v.waterlevel == 3) || (pev->waterlevel == 3 && !tr.pHit->v.waterlevel)))
		{
			if (tr.pHit->v.takedamage == DAMAGE_AIM)
				m_fOnTarget = TRUE;

			return m_vecAutoAim;
		}
	}

	for (int i = 1; i < gpGlobals->maxEntities; i++, pEdict++)
	{
		if (pEdict->free)
			continue;

		if (pEdict->v.takedamage != DAMAGE_AIM)
			continue;

		if (pEdict == edict())
			continue;

		if (!g_pGameRules->ShouldAutoAim(this, pEdict))
			continue;

		CBaseEntity *pEntity = Instance(pEdict);

		if (!pEntity)
			continue;

		if ((pev->waterlevel != 3 && pEntity->pev->waterlevel == 3) || (pev->waterlevel == 3 && !pEntity->pev->waterlevel))
			continue;

		Vector center = pEntity->BodyTarget(vecSrc);
		Vector dir = (center - vecSrc).Normalize();

		if (DotProduct(dir, gpGlobals->v_forward) < 0)
			continue;

		float dot = fabs(DotProduct(dir, gpGlobals->v_right)) + fabs(DotProduct(dir, gpGlobals->v_up)) * 0.5;
		dot *= 1 + 0.2 * ((center - vecSrc).Length() / flDist);

		if (dot > bestdot)
			continue;

		UTIL_TraceLine(vecSrc, center, dont_ignore_monsters, edict(), &tr);

		if (tr.flFraction != 1 && tr.pHit != pEdict)
			continue;

		if (IRelationship(pEntity) < 0)
		{
			if (!pEntity->IsPlayer() && !g_pGameRules->IsDeathmatch())
				continue;
		}

		bestdot = dot;
		bestent = pEdict;
		bestdir = dir;
	}

	if (bestent)
	{
		bestdir = UTIL_VecToAngles(bestdir);
		bestdir.x = -bestdir.x;
		bestdir = bestdir - pev->v_angle - pev->punchangle;

		if (bestent->v.takedamage == DAMAGE_AIM)
			m_fOnTarget = TRUE;

		return bestdir;
	}

	return Vector(0, 0, 0);
}

void CBasePlayer::ResetAutoaim(void)
{
	if (m_vecAutoAim.x || m_vecAutoAim.y)
	{
		m_vecAutoAim = Vector(0, 0, 0);
		SET_CROSSHAIRANGLE(ENT(pev), 0, 0);
	}

	m_fOnTarget = FALSE;
}

void CBasePlayer::SetCustomDecalFrames(int nFrames)
{
	if (nFrames > 0 && nFrames < 8)
		m_nCustomSprayFrames = nFrames;
	else
		m_nCustomSprayFrames = -1;
}

int CBasePlayer::GetCustomDecalFrames(void)
{
	return m_nCustomSprayFrames;
}

void CBasePlayer::DropPlayerItem(const char *pszItemName)
{
	if (!strlen(pszItemName))
		pszItemName = NULL;

	for (int i = 0; i < MAX_ITEM_TYPES; i++)
	{
		CBasePlayerItem *pWeapon = m_rgpPlayerItems[i];

		while (pWeapon)
		{
			if (pszItemName)
			{
				if (!strcmp(pszItemName, STRING(pWeapon->pev->classname)))
					break;
			}
			else if (pWeapon == m_pActiveItem)
				break;

			pWeapon = pWeapon->m_pNext;
		}

		if (pWeapon)
		{
			if (!pWeapon->CanDrop())
			{
				ClientPrint(pev, HUD_PRINTCENTER, "#Weapon_Cannot_Be_Dropped");
				continue;
			}

			g_pGameRules->GetNextBestWeapon(this, pWeapon);
			UTIL_MakeVectors(pev->angles);
			pev->weapons &= ~(1 << pWeapon->m_iId);

			if (pWeapon->iItemSlot() == WEAPON_SLOT_PRIMARY)
				m_bHasPrimaryWeapon = FALSE;

			CWeaponBox *pWeaponBox = (CWeaponBox *)Create("weaponbox", pev->origin + gpGlobals->v_forward * 10, pev->angles, edict());
			pWeaponBox->pev->angles.x = 0;
			pWeaponBox->pev->angles.z = 0;
			pWeaponBox->SetThink(&CBaseEntity::SUB_Remove);
			pWeaponBox->pev->nextthink = gpGlobals->time + 60;
			pWeaponBox->pev->velocity = gpGlobals->v_forward * 300 + gpGlobals->v_forward * 100;
			pWeaponBox->m_flGivePercent = 50;

			const char *modelname = GetWeaponModelName(pWeapon->m_iId);

			if (modelname)
				SET_MODEL(ENT(pWeaponBox->pev), modelname);

			if(!strcmp(modelname, "models/CKF_III/wp_group_rf.mdl"))
			{
				pWeaponBox->pev->sequence = GetWeaponModelSequence(pWeapon->m_iId);
				pWeaponBox->pev->body = GetWeaponModelBody(pWeapon->m_iId);
			}

			return;
		}
	}
}

void CBasePlayer::DropPrimary(void)
{
	while (m_rgpPlayerItems[WEAPON_SLOT_PRIMARY])
		DropPlayerItem(STRING(m_rgpPlayerItems[WEAPON_SLOT_PRIMARY]->pev->classname));
}
//ckf needupdate
int CBasePlayer::GetPrimary(void)
{
	return GetClassPrimary(m_iClass);
}

int CBasePlayer::GetSecondary(void)
{
	return GetClassSecondary(m_iClass);
}

void CBasePlayer::DropPistol(void)
{
	while (m_rgpPlayerItems[WEAPON_SLOT_PISTOL])
		DropPlayerItem(STRING(m_rgpPlayerItems[WEAPON_SLOT_PISTOL]->pev->classname));
}

void CBasePlayer::ThrowWeapon(char *pszWeaponName)
{
	for (int i = 0; i < MAX_WEAPON_SLOTS; i++)
	{
		CBasePlayerItem *pWeapon = m_rgpPlayerItems[i];

		if (!pWeapon)
			break;

		if (!strcmp(pszWeaponName, STRING(pWeapon->pev->classname)))
		{
			DropPlayerItem(pszWeaponName);
			break;
		}

		pWeapon = pWeapon->m_pNext;
	}
}

void CBasePlayer::SwitchSlotWeapon(int iSlot)
{
	if(iSlot < 1 || iSlot > 5)
		return;

	CBasePlayerItem *pWeapon = m_rgpPlayerItems[iSlot];

	if(!pWeapon)
		return;

	if(m_pActiveItem == pWeapon)
		return;

	SwitchWeapon(pWeapon);
}

BOOL CBasePlayer::HasPlayerItem(CBasePlayerItem *pCheckItem)
{
	CBasePlayerItem *pItem = m_rgpPlayerItems[pCheckItem->iItemSlot()];

	while (pItem)
	{
		if (FClassnameIs(pItem->pev, STRING(pCheckItem->pev->classname)))
			return TRUE;

		pItem = pItem->m_pNext;
	}

	return FALSE;
}

BOOL CBasePlayer::HasNamedPlayerItem(const char *pszItemName)
{
	for (int i = 0; i < MAX_ITEM_TYPES; i++)
	{
		CBasePlayerItem *pItem = m_rgpPlayerItems[i];

		while (pItem)
		{
			if (!strcmp(pszItemName, STRING(pItem->pev->classname)))
				return TRUE;

			pItem = pItem->m_pNext;
		}
	}

	return FALSE;
}

void CBasePlayer::SwitchTeam(void)
{
	//char *model;
	int OldTeam = m_iTeam;

	if (m_iTeam == TEAM_CT)
	{
		m_iTeam = TEAM_TERRORIST;

	}
	else if (m_iTeam == TEAM_TERRORIST)
	{
		m_iTeam = TEAM_CT;
	}

	SetPlayerModel();

	MESSAGE_BEGIN(MSG_ALL, gmsgTeamInfo);
	WRITE_BYTE(ENTINDEX(edict()));
	WRITE_STRING(GetTeam(m_iTeam));
	MESSAGE_END();

	if (m_iTeam != TEAM_UNASSIGNED)
		SetScoreboardAttributes();

	const char *name;

	if (pev->netname)
	{
		name = STRING(pev->netname);

		if (!*name)
			name = "<unconnected>";
	}
	else
		name = "<unconnected>";

	if (m_iTeam == TEAM_TERRORIST)
		UTIL_ClientPrintAll(HUD_PRINTNOTIFY, "#Game_join_terrorist_auto", name);
	else
		UTIL_ClientPrintAll(HUD_PRINTNOTIFY, "#Game_join_ct_auto", name);

	UTIL_LogPrintf("\"%s<%i><%s><%s>\" joined team \"%s\" (auto)\n", STRING(pev->netname), GETPLAYERUSERID(edict()), GETPLAYERAUTHID(edict()), GetTeam(OldTeam), GetTeam(m_iTeam));
}

BOOL CBasePlayer::SwitchWeapon(CBasePlayerItem *pWeapon)
{
	if (!pWeapon->CanDeploy())
		return FALSE;

	ResetAutoaim();

	if (m_pActiveItem)
	{
		if(!m_pActiveItem->CanHolster())
			return FALSE;

		m_pLastItem = m_pActiveItem;
		m_pActiveItem->Holster();
	}

	m_pActiveItem = pWeapon;
	pWeapon->Deploy();

	if (pWeapon->m_pPlayer)
		pWeapon->m_pPlayer->ResetMaxSpeed();

	return TRUE;
}

BOOL CBasePlayer::ShouldDoLargeFlinch(int hitgroup, int weaponId)
{
	if (pev->flags & FL_DUCKING)
		return FALSE;

	if (hitgroup != HITGROUP_LEFTLEG && hitgroup != HITGROUP_RIGHTLEG)
	{
		switch (weaponId)
		{
/*			case WEAPON_SCOUT:
			case WEAPON_AUG:
			case WEAPON_SG550:
			case WEAPON_AWP:
			case WEAPON_M3:
			case WEAPON_M4A1:
			case WEAPON_G3SG1:
			case WEAPON_DEAGLE:
			case WEAPON_SG552:*/
			case WEAPON_SNIPERIFLE: return TRUE;
		}
	}

	return FALSE;
}

BOOL CBasePlayer::IsArmored(int hitgroup)
{
	if (m_iKevlar == 0)
		return FALSE;

	switch (hitgroup)
	{
		case HITGROUP_HEAD:
		{
			if (m_iKevlar == 2)
				return TRUE;

			return FALSE;
		}

		case HITGROUP_GENERIC:
		case HITGROUP_CHEST:
		case HITGROUP_STOMACH:
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM: return TRUE;
	}

	return FALSE;
}

void CBasePlayer::MoveToNextIntroCamera(void)
{
	m_pIntroCamera = UTIL_FindEntityByClassname(m_pIntroCamera, "trigger_camera");

	if (!m_pIntroCamera)
		m_pIntroCamera = UTIL_FindEntityByClassname(NULL, "trigger_camera");

	CBaseEntity *pTarget = UTIL_FindEntityByTargetname(NULL, STRING(m_pIntroCamera->pev->target));

	if (pTarget)
	{
		Vector vecDir = (pTarget->pev->origin - m_pIntroCamera->pev->origin).Normalize();
		Vector vecGoal = UTIL_VecToAngles(vecDir);
		vecGoal.x = -vecGoal.x;
		UTIL_SetOrigin(pev, m_pIntroCamera->pev->origin);

		pev->angles = vecGoal;
		pev->v_angle = pev->angles;
		pev->velocity = g_vecZero;
		pev->punchangle = g_vecZero;
		pev->fixangle = TRUE;
		pev->view_ofs = g_vecZero;
		m_fIntroCamTime = gpGlobals->time + 6;
	}
	else
		m_pIntroCamera = NULL;
}

class CDeadHEV : public CBaseMonster
{
public:
	void Spawn(void);
	void KeyValue(KeyValueData *pkvd);
	int Classify(void) { return CLASS_HUMAN_MILITARY; }

public:
	int m_iPose;
	static char *m_szPoses[4];
};

char *CDeadHEV::m_szPoses[] = { "deadback", "deadsitting", "deadstomach", "deadtable" };

void CDeadHEV::KeyValue(KeyValueData *pkvd)
{
	if (!strcmp(pkvd->szKeyName, "pose"))
	{
		m_iPose = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseMonster::KeyValue(pkvd);
};

LINK_ENTITY_TO_CLASS(monster_hevsuit_dead, CDeadHEV);

void CDeadHEV::Spawn(void)
{
	PRECACHE_MODEL("models/player.mdl");
	SET_MODEL(ENT(pev), "models/player.mdl");

	pev->effects = 0;
	pev->yaw_speed = 8;
	pev->sequence = 0;
	pev->body = 0;//1;
	m_bloodColor = BLOOD_COLOR_RED;
	pev->sequence = LookupSequence(m_szPoses[m_iPose]);

	if (pev->sequence == -1)
	{
		ALERT(at_console, "Dead hevsuit with bad pose\n");
		pev->sequence = 0;
		pev->effects = EF_BRIGHTFIELD;
	}

	pev->health = 8;
	MonsterInitDead();
}

class CStripWeapons : public CPointEntity
{
public:
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
};

LINK_ENTITY_TO_CLASS(player_weaponstrip, CStripWeapons);

void CStripWeapons::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	CBasePlayer *pPlayer = NULL;

	if (pActivator && pActivator->IsPlayer())
		pPlayer = (CBasePlayer *)pActivator;
	else if (!g_pGameRules->IsDeathmatch())
		pPlayer = (CBasePlayer *)Instance(INDEXENT(1));

	if (pPlayer)
		pPlayer->RemoveAllItems(FALSE);
}

class CRevertSaved : public CPointEntity
{
public:
	void KeyValue(KeyValueData *pkvd);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

public:
	void EXPORT MessageThink(void);
	void EXPORT LoadThink(void);

public:
	inline float Duration(void) { return pev->dmg_take; }
	inline float HoldTime(void) { return pev->dmg_save; }
	inline float MessageTime(void) { return m_messageTime; }
	inline float LoadTime(void) { return m_loadTime; }
	inline void SetDuration(float duration) { pev->dmg_take = duration; }
	inline void SetHoldTime(float hold) { pev->dmg_save = hold; }
	inline void SetMessageTime(float time) { m_messageTime = time; }
	inline void SetLoadTime(float time) { m_loadTime = time; }

public:
	float m_messageTime;
	float m_loadTime;
};

LINK_ENTITY_TO_CLASS(player_loadsaved, CRevertSaved);

void CRevertSaved::KeyValue(KeyValueData *pkvd)
{
	if (!strcmp(pkvd->szKeyName, "duration"))
	{
		SetDuration(atof(pkvd->szValue));
		pkvd->fHandled = TRUE;
	}
	else if (!strcmp(pkvd->szKeyName, "holdtime"))
	{
		SetHoldTime(atof(pkvd->szValue));
		pkvd->fHandled = TRUE;
	}
	else if (!strcmp(pkvd->szKeyName, "messagetime"))
	{
		SetMessageTime(atof(pkvd->szValue));
		pkvd->fHandled = TRUE;
	}
	else if (!strcmp(pkvd->szKeyName, "loadtime"))
	{
		SetLoadTime(atof(pkvd->szValue));
		pkvd->fHandled = TRUE;
	}
	else
		CPointEntity::KeyValue(pkvd);
}

void CRevertSaved::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	UTIL_ScreenFadeAll(pev->rendercolor, Duration(), HoldTime(), (int)pev->renderamt, FFADE_OUT);
	pev->nextthink = gpGlobals->time + MessageTime();
	SetThink(&CRevertSaved::MessageThink);
}

void CRevertSaved::MessageThink(void)
{
	UTIL_ShowMessageAll(STRING(pev->message));
	float nextThink = LoadTime() - MessageTime();

	if (nextThink > 0)
	{
		pev->nextthink = gpGlobals->time + nextThink;
		SetThink(&CRevertSaved::LoadThink);
	}
	else
		LoadThink();
}

void CRevertSaved::LoadThink(void)
{
	if (!gpGlobals->deathmatch)
		SERVER_COMMAND("reload\n");
}

class CInfoIntermission : public CPointEntity
{
public:
	void Spawn(void);
	void Think(void);
};

void CInfoIntermission::Spawn(void)
{
	UTIL_SetOrigin(pev, pev->origin);

	pev->solid = SOLID_NOT;
	pev->effects = EF_NODRAW;
	pev->v_angle = g_vecZero;
	pev->nextthink = gpGlobals->time + 2;
}

void CInfoIntermission::Think(void)
{
	edict_t *pTarget = FIND_ENTITY_BY_TARGETNAME(NULL, STRING(pev->target));

	if (!FNullEnt(pTarget))
	{
		pev->v_angle = UTIL_VecToAngles((pTarget->v.origin - pev->origin).Normalize());
		pev->v_angle.x = -pev->v_angle.x;
	}
}

LINK_ENTITY_TO_CLASS(info_intermission, CInfoIntermission);

void CBasePlayer::StudioEstimateGait(void)
{
	float dt;
	vec3_t est_velocity;

	dt = gpGlobals->frametime;

	if (dt < 0)
		dt = 0;
	else if (dt > 1)
		dt = 1;

	if (dt == 0) 
	{
		m_flGaitMovement = 0;
		return;
	}

	est_velocity[0] = pev->origin[0] - m_vecPrevGaitOrigin[0];
	est_velocity[1] = pev->origin[1] - m_vecPrevGaitOrigin[1];
	est_velocity[2] = pev->origin[2] - m_vecPrevGaitOrigin[2];
	m_vecPrevGaitOrigin[0] = pev->origin[0];
	m_vecPrevGaitOrigin[1] = pev->origin[1];
	m_vecPrevGaitOrigin[2] = pev->origin[2];
	m_flGaitMovement = sqrt(est_velocity[0] * est_velocity[0] + est_velocity[1] * est_velocity[1] + est_velocity[2] * est_velocity[2]); 

	if (dt <= 0 || m_flGaitMovement / dt < 5)
	{
		m_flGaitMovement = 0;
		est_velocity[0] = est_velocity[1] = 0;
	}

	if (est_velocity[0] == 0 && est_velocity[1] == 0)
	{
		float flYawDiff = pev->angles[1] - m_flGaitYaw;
		float flYaw = flYawDiff;
		flYawDiff = flYawDiff - (int)(flYawDiff / 360) * 360;

		if (flYawDiff > 180)
			flYawDiff -= 360;

		if (flYawDiff < -180)
			flYawDiff += 360;

		if (flYaw < -180)
			flYaw += 360;
		else if (flYaw > 180)
			flYaw -= 360;

		if (flYaw > -5 && flYaw < 5)
			m_flYawModifier = 0.05;

		if (flYaw < -90 || flYaw > 90)
			m_flYawModifier = 3.5;

		if (dt < 0.25)
			flYawDiff *= dt * m_flYawModifier;
		else
			flYawDiff *= dt;

		if (abs(flYawDiff) < 0.1)
			flYawDiff = 0;

		m_flGaitYaw += flYawDiff;
		m_flGaitYaw -= (int)(m_flGaitYaw / 360) * 360;
		m_flGaitMovement = 0;
	}
	else
	{
		m_flGaitYaw = (atan2(est_velocity[1], est_velocity[0]) * 180 / M_PI);

		if (m_flGaitYaw > 180)
			m_flGaitYaw = 180;

		if (m_flGaitYaw < -180)
			m_flGaitYaw = -180;
	}
}

void CBasePlayer::CalculatePitchBlend(void)
{
	int iBlend;
	float temp;

	temp = (int)(pev->angles[0] * 3);

	if (temp <= -45)
		iBlend = 255;
	else if (temp < 45)
		iBlend = ((45.0 - temp) / (45.0 + 45)) * 255;
	else
		iBlend = 0;

	pev->blending[1] = iBlend;
	m_flPitch = iBlend;
}

void CBasePlayer::CalculateYawBlend(void)
{
	float dt;
	float flYaw;
	float maxyaw;
	float blend_yaw;

	dt = gpGlobals->frametime;

	if (dt < 0)
		dt = 0;
	else if (dt > 1)
		dt = 1;

	StudioEstimateGait();

	maxyaw = 255.0;
	flYaw = pev->angles[1] - m_flGaitYaw;

	if (flYaw < -180)
		flYaw += 360;
	else if (flYaw > 180)
		flYaw -= 360;

	if (m_flGaitMovement != 0)
	{
		if (flYaw > 120)
		{
			m_flGaitYaw -= 180;
			m_flGaitMovement = -m_flGaitMovement;
			flYaw -= 180;
		}
		else if (flYaw < -120)
		{
			m_flGaitYaw += 180;
			m_flGaitMovement = -m_flGaitMovement;
			flYaw += 180;
		}
	}

	flYaw = (flYaw / 90) * 128.0 + 127.0;

	if (flYaw > 255)
		flYaw = 255;
	else if (flYaw < 0)
		flYaw = 0;

	blend_yaw = maxyaw - flYaw;
	pev->blending[0] = (int)(blend_yaw);
	m_flYaw = blend_yaw;
}

void CBasePlayer::StudioProcessGait(void)
{
	mstudioseqdesc_t *pseqdesc;
	float dt = gpGlobals->frametime;

	if (dt < 0)
		dt = 0;
	else if (dt > 1.0)
		dt = 1;

	CalculateYawBlend();
	CalculatePitchBlend();

	void *model = GET_MODEL_PTR(ENT(pev));

	if (!model)
		return;

	studiohdr_t *pstudiohdr = (studiohdr_t *)model;

	pseqdesc = (mstudioseqdesc_t *)((byte *)pstudiohdr + pstudiohdr->seqindex) + pev->gaitsequence;

	if (pseqdesc->linearmovement[0] > 0)
		m_flGaitFrame += (m_flGaitMovement / pseqdesc->linearmovement[0]) * pseqdesc->numframes;
	else
		m_flGaitFrame += pseqdesc->fps * dt * pev->framerate;

	m_flGaitFrame -= (int)(m_flGaitFrame / pseqdesc->numframes) * pseqdesc->numframes;

	if (m_flGaitFrame < 0)
		m_flGaitFrame += pseqdesc->numframes;

}

float GetPlayerPitch(const edict_t *pEdict)
{
	entvars_t *pev = VARS((edict_t *)pEdict);
	CBasePlayer *pPlayer = (CBasePlayer *)CBaseEntity::Instance(pev);

	if (!pPlayer)
		return 0;

	return pPlayer->m_flPitch;
}

float GetPlayerYaw(const edict_t *pEdict)
{
	entvars_t *pev = VARS((edict_t *)pEdict);
	CBasePlayer *pPlayer = (CBasePlayer *)CBaseEntity::Instance(pev);

	if (!pPlayer)
		return 0;

	return pPlayer->m_flYaw;
}

int GetPlayerGaitsequence(const edict_t *pEdict)
{
	entvars_t *pev = VARS((edict_t *)pEdict);
	CBasePlayer *pPlayer = (CBasePlayer *)CBaseEntity::Instance(pev);

	if (!pPlayer)
		return 0;

	return pPlayer->m_iGaitSequence;
}

void CBasePlayer::SpawnClientSideCorpse(void)
{
	m_canSwitchObserverModes = true;
}

void CBasePlayer::SetPrefsFromUserinfo(char *infobuffer)
{
	const char *autowepswitch = g_engfuncs.pfnInfoKeyValue(infobuffer, "_cl_autowepswitch");

	if (strcmp(autowepswitch, ""))
		m_iAutoWepSwitch = atoi(autowepswitch);
	else
		m_iAutoWepSwitch = 1;

	const char *autoreload = g_engfuncs.pfnInfoKeyValue(infobuffer, "_cl_autoreload");

	if (strcmp(autoreload, ""))
		m_bAutoReload = (atoi(autoreload) > 0) ? true : false;
	else
		m_bAutoReload = true;

	const char *vguimenu = g_engfuncs.pfnInfoKeyValue(infobuffer, "_vgui_menus");

	if (strcmp(vguimenu, ""))
		m_bVGUIMenus = atoi(vguimenu) != 0;
	else
		m_bVGUIMenus = TRUE;

	const char *ah = g_engfuncs.pfnInfoKeyValue(infobuffer, "_ah");

	if (strcmp(ah, ""))
		m_bShowHints = atoi(ah) != 0;
	else
		m_bShowHints = TRUE;

	const char *fov = g_engfuncs.pfnInfoKeyValue(infobuffer, "_fov");

	if (strcmp(fov, ""))
		m_iDefaultFOV = min(max(atoi(fov), 65), 90);
	else
		m_iDefaultFOV = 90;

	if(m_iFOV >= 65)
		m_iFOV = m_iDefaultFOV;
}

int CBasePlayer::GetBoostMaxHealth(void)
{
	float flBoostMax = pev->max_health * 1.5f;

	int iRoundDown = floor( flBoostMax / 5 );
	iRoundDown = iRoundDown * 5;

	return iRoundDown;
}

/*
iType -1:no random crit, 0:rocket shotgun 1:minigun,flamethrower, 2=melee
*/

int CBasePlayer::GetCriticalFire(int iType, unsigned int iRandSeed)
{
	int iCrit = 0;

	if(m_Cond.CritBoost.m_iStatus)
		iCrit += 2;

	float fRand = 0;
	switch(iType)
	{
		case 0:
		{
			fRand = UTIL_SharedRandomFloat(iRandSeed, 0.0, 100.0);
			if(fRand <= m_fCritChance) iCrit += 2;
			break;
		}
		case 1:
		{
			if(m_bCritBuff)
			{
				iCrit += 2;
			}
			break;
		}
		case 2:
		{
			float fCritChance = 15.0f + 50.0f*m_iDmgDone_Recent/800.0f;
			if(fCritChance > 65.0f) fCritChance = 65.0f;
			fRand = UTIL_SharedRandomFloat(iRandSeed, 0.0, 100.0);
			if(fRand <= fCritChance) iCrit += 2;
			//ALERT(at_console, "fMCritChance = %.2f\n", fCritChance);
			break;
		}
	}
	return iCrit;
}

void CBasePlayer::CritChance_Think()
{
	if(m_fCritChanceTimer >= gpGlobals->time)
		return;
	if(pev->deadflag != DEAD_NO)
		return;

	m_fCritChance = 2.0f + 10.0f*m_iDmgDone_Recent/800.0f;

	if(m_fCritChance > 12.0f)
		m_fCritChance = 12.0f;

	if(m_fDmgDone_Decesase > 0)
		m_iDmgDone_Recent -= m_fDmgDone_Decesase;

	if(m_iDmgDone_Recent)
		m_fDmgDone_Decesase = m_iDmgDone_Recent / 20.0f;

	m_fCritChanceTimer = gpGlobals->time + 1.0;
}

void CBasePlayer::CritBuff_Think()
{
	if(m_fCritBuffTimer >= gpGlobals->time)
		return;
	if(pev->deadflag != DEAD_NO)
		return;
	if(g_pGameRules->m_iRoundStatus == ROUND_END)
		return;

	if(m_bCritBuff)
	{
		m_bCritBuff = false;
	}
	else
	{
		float fRand = RANDOM_FLOAT(1.0, 100.0);
		if(fRand <= m_fCritChance)
		{
			m_bCritBuff = true;
			m_fCritBuffTimer = gpGlobals->time + RANDOM_FLOAT(4.0, 6.0);
			return;
		}

	}
	m_fCritBuffTimer = gpGlobals->time + 1.0;
}

void CBasePlayer::Respawn_Think()
{
	if(!m_iClass && !m_iNewClass)
		return;
	if(!m_bIsRespawning)
		return;
	if(m_fRespawnCounter >= gpGlobals->time)
		return;
	if(pev->deadflag == DEAD_NO)
		return;
	if(g_pGameRules->m_iRoundStatus == ROUND_END)
		return;

	int iDelta = int( ceil(m_fRespawnTimer - gpGlobals->time) );
 	if(iDelta >= 0)
	{
		SendRespawnTime(this, iDelta);
	}
	else
	{
		RoundRespawn();
	}
	m_fRespawnCounter = gpGlobals->time + 1.0;
}

void CBasePlayer::Respawn_Start()
{
	if(!m_iClass && !m_iNewClass)
		return;
	if(g_pGameRules->m_iRoundStatus == ROUND_END)
		return;
	if(g_pGameRules->m_iRespawnDisabled == m_iTeam || g_pGameRules->m_iRespawnDisabled >= 3)
		return;
	if(m_iTeam != TEAM_RED && m_iTeam != TEAM_BLU)
		return;
	if(m_iJoiningState != JOINED)
		return;
	if(m_bIsRespawning)
		return;

	float flWave, flWaveThink, flMax, flMin, flRespawn;

	if(m_iTeam == TEAM_RED)
	{
		flWave = g_pGameRules->m_fRedWaveTime;
		flWaveThink = g_pGameRules->m_fRedWaveThink;
		flMax = g_pGameRules->m_fRedMaxRespawn;
		flMin = g_pGameRules->m_fRedMinRespawn;
	}
	else
	{
		flWave = g_pGameRules->m_fBluWaveTime;
		flWaveThink = g_pGameRules->m_fBluWaveThink;
		flMax = g_pGameRules->m_fBluMaxRespawn;
		flMin = g_pGameRules->m_fBluMinRespawn;
	}

	flRespawn = flWaveThink - gpGlobals->time;

	if(flRespawn < flMin)
	{
		flRespawn = flWaveThink + flWave;
	}
	if(g_pGameRules->m_iRespawnPlayerNum)
	{
		int iPlayers = (m_iTeam == TEAM_RED) ? g_pGameRules->m_iNumTerrorist : g_pGameRules->m_iNumCT;
		if(iPlayers < g_pGameRules->m_iRespawnPlayerNum) flRespawn -= g_pGameRules->m_fRespawnPlayerFactor * (g_pGameRules->m_iRespawnPlayerNum-iPlayers);
	}
	if(flRespawn < flMin)
	{
		flRespawn = flMin;
	}
	if(flRespawn > flMax)
	{
		flRespawn = flMax;
	}
	m_fRespawnTimer = gpGlobals->time + flRespawn;
	m_fRespawnCounter = gpGlobals->time + 0.1;
	m_bIsRespawning = true;
}

float CBasePlayer::GetPlayerMaxSpeed(void)
{
	return GetClassMaxSpeed(m_iClass);
}

void CBasePlayer::Cloak_Begin(void)
{
	if(m_iClass != CLASS_SPY) 
		return;
	if(m_iCloak != CLOAK_NO)
		return;
	if(m_flCloakEnergy <= 5)
		return;

	m_iCloak = CLOAK_BEGIN;
	m_fCloakTimer = gpGlobals->time + CloakBegin_Duration;
	m_flCloakEnergyTimer = gpGlobals->time + 0.1;

	MESSAGE_BEGIN(MSG_ONE, gmsgDrawFX, NULL, pev);
	WRITE_BYTE(FX_CLOAKBEGIN);
	MESSAGE_END();

	EMIT_SOUND(ENT(pev), CHAN_STATIC, "CKF_III/spy_cloak.wav", 1.0, 1.8);
}

void CBasePlayer::Cloak_Think(void)
{
	if(m_iClass != CLASS_SPY)
		return;
	if(m_iCloak == CLOAK_NO)
	{
		if(m_flCloakEnergy < 100 && m_flCloakEnergyTimer < gpGlobals->time)
		{
			m_flCloakEnergy += 0.2985f;

			if(m_flCloakEnergy > 100)
				m_flCloakEnergy = 100;

			m_flCloakEnergyTimer = gpGlobals->time + 0.1;
		}
		return;
	}
	if(m_flCloakEnergy <= 0 && m_iCloak == CLOAK_YES)
	{
		m_fCloakTimer = gpGlobals->time;
		Cloak_Stop();
		return;
	}
	if((m_iCloak == CLOAK_YES || m_iCloak == CLOAK_BEGIN) && m_flCloakEnergyTimer < gpGlobals->time)
	{
		m_flCloakEnergy -= 1;

		if(m_flCloakEnergy < 0)
			m_flCloakEnergy = 0;

		m_flCloakEnergyTimer = gpGlobals->time + 0.1;
		pev->flTimeStepSound = 999;
	}
	if(m_iCloak == CLOAK_BEGIN)
	{
		//pev->rendermode = kRenderTransTexture;
		pev->renderamt = 255 * (m_fCloakTimer - gpGlobals->time) / CloakBegin_Duration;
		if(m_fCloakTimer <= gpGlobals->time)
		{
			m_iCloak = CLOAK_YES;
			pev->renderamt = 0;
		}
	}
	else if(m_iCloak == CLOAK_STOP)
	{
		//pev->rendermode = kRenderTransTexture;
		pev->renderamt = 255 * ( 1 - (m_fCloakTimer - gpGlobals->time) / CloakStop_Duration );
		if(m_fCloakTimer <= gpGlobals->time)
		{
			m_iCloak = CLOAK_NO;
			pev->renderamt = 255;
			pev->rendermode = kRenderNormal;
		}
	}
}

void CBasePlayer::Cloak_Stop(void)
{
	if(m_iClass != CLASS_SPY)
		return;
	if(m_iCloak != CLOAK_YES)
		return;

	m_iCloak = CLOAK_STOP;
	m_fCloakTimer = gpGlobals->time + CloakStop_Duration;

	MESSAGE_BEGIN(MSG_ONE, gmsgDrawFX, NULL, pev);
	WRITE_BYTE(FX_CLOAKSTOP);
	MESSAGE_END();

	EMIT_SOUND(ENT(pev), CHAN_STATIC, "CKF_III/spy_uncloak.wav", 0.8, 2.4);
}

BOOL CBasePlayer::PlayerCanAttack(void)
{
	if(!m_bAllowAttack)
		return FALSE;
	if(m_iCloak != CLOAK_NO)
		return FALSE;
	if(g_pGameRules->m_bFreezePeriod)
		return FALSE;
	return TRUE;
}

void CBasePlayer::ClearSticky(void)
{
	CBaseEntity *pEntity = NULL;
	while ((pEntity = UTIL_FindEntityByClassname(pEntity, "pj_sticky")) != NULL)
	{
		if(pEntity->pev->owner != edict()) continue;
		CSticky *pSticky = (CSticky *)pEntity;
		pSticky->Killed(pev, -1);
	}
}

void CBasePlayer::Health_Think(void)
{
	int iHealerCount = 0;
	float flHealAmount = 0;
	CBasePlayer *pPlayer;

	m_pHealer = NULL;
	m_iHealerCount = 0;

	for(int i = 1; i < gpGlobals->maxClients; ++i)
	{
		pPlayer = (CBasePlayer *)UTIL_PlayerByIndex(i);
		if (!pPlayer || !FNullEnt(pPlayer->pev)) continue;
		if(pPlayer->IsAlive() && pPlayer->m_iClass == CLASS_MEDIC)
		{
			if(pPlayer->m_pActiveItem && pPlayer->m_pActiveItem->iItemSlot() == WEAPON_SLOT_SECONDARY) 
			{
				CMedigun *gun = dynamic_cast<CMedigun *>(pPlayer->m_pActiveItem);
				if(gun)
				{
					if(gun->m_bHealing && gun->m_pTarget == this)
					{
						iHealerCount ++;
						if(pPlayer->m_flHealAmount > flHealAmount)
						{
							flHealAmount = pPlayer->m_flHealAmount;
							m_pHealer = pPlayer;
						}
					}
				}
			}
		}
	}

	if(!iHealerCount)
	{
		if(m_iDisguise == DISGUISE_YES && m_iDisguiseHealth > m_iDisguiseMaxHealth && m_flDisguiseHealthReduce < gpGlobals->time)
		{
			m_iDisguiseHealth = max(m_iDisguiseHealth - m_iDisguiseMaxHealth*0.0025, m_iDisguiseMaxHealth);
			m_flDisguiseHealthReduce = gpGlobals->time + 0.1;
		}
		if(pev->health > pev->max_health && m_flHealthReduce < gpGlobals->time)
		{
			pev->health = max(pev->health - pev->max_health*0.0025, pev->max_health);
			m_flHealthReduce = gpGlobals->time + 0.1;
		}
	}
	if(m_iClass == CLASS_MEDIC && pev->health < pev->max_health && m_flHealthRegen < gpGlobals->time)
	{
		float flRegen = 0;
		if(gpGlobals->time - m_flLastDamagedTime > 10)
			flRegen = 6;
		else
			flRegen = 3.0*(gpGlobals->time - m_flLastDamagedTime)/10.0+3.0;
		if(pev->health < pev->max_health)
			pev->health = min(pev->health + flRegen, pev->max_health);
		m_flHealthRegen = gpGlobals->time + 1;
	}
	if(m_flHealAmount >= 600)
	{
		m_Stats.iHealing ++;
		SendStatsInfo(STATS_HEALING);
		AddPoints(1, FALSE);
		m_flHealAmount = 0;
	}
}

void CBasePlayer::Condition_Clear(void)
{
	m_Cond.Invulnerable.Remove();
	m_Cond.CritBoost.Remove();
	m_Cond.AfterBurn.Remove();
}

void CBasePlayer::Condition_Init(void)
{
	m_Cond.Invulnerable.Init(this);
	m_Cond.CritBoost.Init(this);
	m_Cond.AfterBurn.Init(this);
}

void CBasePlayer::Condition_Think(void)
{
	m_Cond.Invulnerable.Think();
	m_Cond.CritBoost.Think();
	m_Cond.AfterBurn.Think();
}

void CPlayerCondition::Init(CBasePlayer *pPlayer)
{
	m_iStatus = 0;
	m_flDie = 0;
	m_flNextThink = 0;
	m_pPlayer = pPlayer;
}

void CPlayerCondition::Think(void)
{
	if(m_iStatus)
	{
		OnThink();

		if(gpGlobals->time > m_flDie)
		{
			m_iStatus = 0;
			OnRemove();
		}
	}
}

void CPlayerCondition::Remove(void)
{
	if(m_iStatus)
	{
		OnRemove();
		m_iStatus = 0;
	}
}

//invulnerable

void CCondInvulnerable::OnThink(void)
{
	if(gpGlobals->time > m_flNextThink)
	{
		if(m_iStatus >= 2)
		{
			m_pPlayer->pev->effects ^= EF_INVULNERABLE;//XOR so we have this effect switched on and off by turns per 0.33s
			m_flNextThink = gpGlobals->time + 0.33f;
		}
		else
		{
			m_pPlayer->pev->effects |= EF_INVULNERABLE;//we just add this effect per 0.5s
			m_flNextThink = gpGlobals->time + 0.5f;
		}
	}
	if(gpGlobals->time > m_flNextEffect)
	{
		MESSAGE_BEGIN(MSG_BROADCAST, gmsgDrawFX);
		WRITE_BYTE(FX_INVULNPLAYER);
		WRITE_SHORT(m_pPlayer->entindex());
		WRITE_BYTE(m_pPlayer->m_iTeam);
		WRITE_COORD(m_flDie - gpGlobals->time);
		MESSAGE_END();

		m_flNextEffect = gpGlobals->time + 1.0f;
	}
}

void CCondInvulnerable::OnRemove(void)
{
	MESSAGE_BEGIN(MSG_BROADCAST, gmsgDrawFX);
	WRITE_BYTE(FX_INVULNPLAYER);
	WRITE_SHORT(m_pPlayer->entindex());
	WRITE_BYTE(m_pPlayer->m_iTeam);
	WRITE_COORD(0);
	MESSAGE_END();

	m_pPlayer->pev->effects &= ~EF_INVULNERABLE;
}

void CCondInvulnerable::AddToPlayer(float flDuration, qboolean bFlash)
{
	if(!m_iStatus)
	{
		m_iStatus = 1;
		m_flNextThink = gpGlobals->time;
		m_flNextEffect = gpGlobals->time;
	}
	else if(bFlash)
	{
		m_iStatus = 2;
	}
	m_flDie = gpGlobals->time + flDuration;
}

void CBasePlayer::Invulnerable_Add(float flDuration, qboolean bFlash)
{
	m_Cond.Invulnerable.AddToPlayer(flDuration, bFlash);
}

//CritBoost

void CCondCritBoost::OnThink(void)
{
	if(gpGlobals->time > m_flNextThink)
	{
		m_pPlayer->pev->effects |= EF_CRITBOOST;//we just add this effect per 0.5s
		m_flNextThink = gpGlobals->time + 0.5f;
	}
	if(gpGlobals->time > m_flNextEffect)
	{
		MESSAGE_BEGIN(MSG_BROADCAST, gmsgDrawFX);
		WRITE_BYTE(FX_CRITPLAYERWEAPON);
		WRITE_SHORT(m_pPlayer->entindex());
		WRITE_BYTE(m_pPlayer->m_iTeam);
		WRITE_COORD(m_flDie - gpGlobals->time);
		MESSAGE_END();

		m_flNextEffect = gpGlobals->time + 1.0f;
	}
}

void CCondCritBoost::OnRemove(void)
{
	MESSAGE_BEGIN(MSG_BROADCAST, gmsgDrawFX);
	WRITE_BYTE(FX_CRITPLAYERWEAPON);
	WRITE_SHORT(m_pPlayer->entindex());
	WRITE_BYTE(m_pPlayer->m_iTeam);
	WRITE_COORD(0);
	MESSAGE_END();

	m_pPlayer->pev->effects &= ~EF_CRITBOOST;
}

void CCondCritBoost::AddToPlayer(float flDuration)
{
	if(!m_iStatus)
	{
		m_iStatus = 1;
		m_flNextThink = gpGlobals->time;
		m_flNextEffect = gpGlobals->time;
	}
	m_flDie = gpGlobals->time + flDuration;
}

void CBasePlayer::CritBoost_Add(float flDuration)
{
	m_Cond.CritBoost.AddToPlayer(flDuration);
}

//AfterBurn

void CCondAfterBurn::OnThink(void)
{
	if(gpGlobals->time > m_flNextThink)
	{
		m_pPlayer->pev->effects |= EF_AFTERBURN;//we just add this effect per 0.5s

		int iCrit = m_pPlayer->GetCriticalHit();
		int iDamage = m_iDamage;
		if(iCrit >= 2)
			iDamage *= 3;
		else if(iCrit)
			iDamage = iDamage * 135 / 100;

		int bTakeDamage;
		if(!m_pevInflictor)
			bTakeDamage = m_pPlayer->TakeDamage(m_pevIgniter, m_pevIgniter, iDamage, DMG_NEVERGIB, iCrit);
		else
			bTakeDamage = m_pPlayer->TakeDamage(m_pevInflictor, m_pevIgniter, iDamage, DMG_NEVERGIB, iCrit);

		if(bTakeDamage)
			m_pPlayer->pev->punchangle.x = 4 * m_pPlayer->GetPunch();

		m_flNextThink = gpGlobals->time + 0.5f;
	}
	if(gpGlobals->time > m_flNextEffect)
	{
		MESSAGE_BEGIN(MSG_BROADCAST, gmsgDrawFX);
		WRITE_BYTE(FX_BURNINGPLAYER);
		WRITE_SHORT(m_pPlayer->entindex());
		WRITE_BYTE(m_pPlayer->m_iTeam);
		WRITE_COORD(m_flDie - gpGlobals->time);
		MESSAGE_END();

		m_flNextEffect = gpGlobals->time + 1.0f;
	}
}

void CCondAfterBurn::OnRemove(void)
{
	m_pPlayer->pev->effects &= ~EF_AFTERBURN;

	MESSAGE_BEGIN(MSG_BROADCAST, gmsgDrawFX);
	WRITE_BYTE(FX_BURNINGPLAYER);
	WRITE_SHORT(m_pPlayer->entindex());
	WRITE_BYTE(m_pPlayer->m_iTeam);
	WRITE_COORD(0);
	MESSAGE_END();
}

void CCondAfterBurn::AddToPlayer(float flDuration, int iDamage, entvars_t *pevIgniter, entvars_t *pevInflictor)
{
	float flRealDuration = (m_pPlayer->m_iClass == CLASS_PYRO) ? flDuration / 5.0f : flDuration;

	if(!m_iStatus)
	{
		m_iStatus = 1;

		m_iDamage = iDamage;
		m_pevIgniter = pevIgniter;
		m_pevInflictor = pevInflictor;

		m_flNextEffect = gpGlobals->time;
		m_flNextThink = gpGlobals->time;
	}
	m_flDie = gpGlobals->time + flRealDuration;
}

void CBasePlayer::AfterBurn_Add(float flDuration, int iDamage, entvars_t *pevIgniter, entvars_t *pevInflictor)
{
	m_Cond.AfterBurn.AddToPlayer(flDuration, iDamage, pevIgniter, pevInflictor);
}

void CBasePlayer::Uber_Think(void)
{
	if(m_iClass != CLASS_MEDIC) return;
	if(!m_iUbercharge) return;
	if(m_flUberThink > gpGlobals->time) return;
	m_flUberThink = gpGlobals->time + 0.1;
	m_fUbercharge -= 1.25f;
	if(m_fUbercharge <= 0)
	{
		m_fUbercharge = 0;
		m_iUbercharge = 0;
		if(m_pActiveItem)
		{
			if(m_pActiveItem->iItemSlot() == 1)
			{
				((CBasePlayerWeapon *)m_pActiveItem)->SendWeaponAnim(0);
			}
		}
		EMIT_SOUND(ENT(pev), CHAN_STATIC, "CKF_III/invulnerable_off.wav", 1.0, ATTN_NORM);		
		m_iUbercharge &= ~UC_INVULNERABLE;
	}
}

BOOL CBasePlayer::ResupplyCase(void)
{
	if(m_flResupplyCase > gpGlobals->time)
		return FALSE;

	if(m_iClass == CLASS_SPY && m_flCloakEnergy < 100) 
	{
		m_flCloakEnergy = 100;
		pev->armorvalue = m_flCloakEnergy;
	}

	if(m_iClass == CLASS_ENGINEER && m_iMetal < 200) 
	{
		int iShit = 200-m_iMetal;
		m_iMetal = 200;

		SendAddMetal(iShit);
	}

	CBasePlayerWeapon *pWeapon = NULL;
	char *pszAmmo = NULL;
	int iMaxAmmo = 0;
	for (int i = WEAPON_SLOT_PRIMARY; i <= WEAPON_SLOT_MELEE; i++)
	{
		pWeapon = (m_rgpPlayerItems[i]) ? (CBasePlayerWeapon *)m_rgpPlayerItems[i] : NULL;
		if(!pWeapon)
			continue;
		if(i <= WEAPON_SLOT_SECONDARY)
		{
			iMaxAmmo = pWeapon->iMaxAmmo1();
			pszAmmo = (char *)pWeapon->pszAmmo1();
			if(pszAmmo && iMaxAmmo) GiveAmmo(iMaxAmmo, pszAmmo, iMaxAmmo);
			(pWeapon)->m_iClip = pWeapon->iMaxClip();
		}
		pWeapon->Reset();
	}

	int iHealthAdded = 0;

	if(pev->health < pev->max_health)
	{
		int iOldHealth = pev->health;
		pev->health = pev->max_health;
		iHealthAdded = pev->health - iOldHealth;
	}

	if(iHealthAdded)
	{
		SendAddHealth(iHealthAdded);
	}

	m_Cond.AfterBurn.Remove();//灭火

	EMIT_SOUND(ENT(pev), CHAN_ITEM, "CKF_III/regenerate.wav", VOL_NORM, ATTN_NORM);
	m_flResupplyCase = gpGlobals->time + 4.0;
	return TRUE;
}

float CBasePlayer::GetKnockBack(void)
{
	switch(m_iClass)
	{
	case CLASS_SCOUT: return 1.1;
	case CLASS_HEAVY: return 0.5;
	case CLASS_SOLDIER: return 0.8;
	case CLASS_PYRO: return 0.9;
	case CLASS_SNIPER: return 1.0;
	case CLASS_MEDIC:return 0.9;
	case CLASS_ENGINEER:return 1.0;
	case CLASS_DEMOMAN:return 0.8;
	case CLASS_SPY:return 1.0;
	}
	return 1.0;
}

float CBasePlayer::GetPunch(void)
{
	switch(m_iClass)
	{
	case CLASS_SCOUT: return 1.2;
	case CLASS_HEAVY: return 0.3;
	case CLASS_SOLDIER: return 0.8;
	case CLASS_PYRO: return 0.8;
	case CLASS_SNIPER: return 0.6;
	case CLASS_MEDIC:return 1.0;
	case CLASS_ENGINEER:return 1.0;
	case CLASS_DEMOMAN:return 0.8;
	case CLASS_SPY:return 1.0;
	}
	return 1.0;
}

void CBasePlayer::PostDeath(void)
{
	Condition_Clear();
}

float CBasePlayer::GetCapRate(void)
{
	switch(m_iClass)
	{
	case CLASS_SCOUT: return 2.0;
	case CLASS_HEAVY: return 1.0;
	case CLASS_SOLDIER: return 1.0;
	case CLASS_PYRO: return 1.0;
	case CLASS_SNIPER: return 1.0;
	case CLASS_MEDIC:return 1.0;
	case CLASS_ENGINEER:return 1.0;
	case CLASS_DEMOMAN:return 1.0;
	case CLASS_SPY:return 1.0;
	}
	return 1.0;
}

bool CBasePlayer::CanCapture(void)
{
	if(m_iCloak != CLOAK_NO) return false;

	if(m_iDisguise != DISGUISE_NO) return false;

	if(m_Cond.Invulnerable.m_iStatus) return false;

	//if(m_Cond.CritBoost.m_iStatus) return false;

	return true;
}

#define DisguiseBegin_Duration (2.0f)

void CBasePlayer::Disguise_Start(int iTeam, int iClass)
{
	if(m_iClass != CLASS_SPY)
		return;
	if(iTeam < 1 || iTeam > 2)
		return;
	if(iClass < CLASS_SCOUT || iClass > CLASS_SPY)
		return;
	if(m_bDisguiseStart && m_iDisguiseNewTeam == iTeam && m_iDisguiseNewClass == iClass)
		return;

	m_iDisguiseNewTeam = iTeam;
	m_iDisguiseNewClass = iClass;

	if(m_iDisguiseNewTeam == m_iDisguiseTeam && m_iDisguiseNewClass == m_iDisguiseClass && m_iDisguise == DISGUISE_YES)
	{
		Disguise_Weapon();
		return;
	}

	m_bDisguiseStart = true;

	m_flDisguiseTimer = gpGlobals->time + DisguiseBegin_Duration;

	EMIT_SOUND(ENT(pev) , CHAN_STATIC, "CKF_III/spy_disguise.wav", 1.0, 2.0);

	if( m_iCloak == CLOAK_NO )
	{
		MESSAGE_BEGIN(MSG_BROADCAST, gmsgDrawFX);
		WRITE_BYTE(FX_DISGUISESMOKE);
		WRITE_SHORT(ENTINDEX(edict()));
		WRITE_BYTE(m_iDisguiseNewTeam);
		MESSAGE_END();
	}
}

void CBasePlayer::Disguise_Weapon(void)
{
	if(m_iClass != CLASS_SPY)
		return;

	int iSlot = 0;
	int iWeaponID = 0;
	if(m_pActiveItem)
		iSlot = m_pActiveItem->iItemSlot();		
	if(iSlot == 1)
		iWeaponID = GetClassPrimary(m_iDisguiseClass);
	else if(iSlot == 2)
		iWeaponID = GetClassSecondary(m_iDisguiseClass);
	else
		iWeaponID = GetClassMelee(m_iDisguiseClass);

	if(m_iDisguiseWeaponID && m_iDisguiseWeaponID == iWeaponID)
		return;

	m_iDisguiseWeaponID = iWeaponID;
	m_iDisguiseWeapon = MODEL_INDEX(GetWeaponModelName(iWeaponID));
	m_iDisguiseWeaponBody = GetWeaponModelBody(iWeaponID);
	if(iSlot == 1)
		strcpy(m_szDisguiseAnimExtention, "shotgun");
	else if(iSlot == 2)
		strcpy(m_szDisguiseAnimExtention, "onehanded");
	else
		strcpy(m_szDisguiseAnimExtention, "knife");
}

void CBasePlayer::Disguise_Think(void)
{
	if(m_iClass != CLASS_SPY)
		return;

	if(!m_bDisguiseStart)
		return;

	if (m_flDisguiseTimer > gpGlobals->time)
		return;

	m_iDisguiseTeam = m_iDisguiseNewTeam;
	m_iDisguiseClass = m_iDisguiseNewClass;

	CBasePlayer *pTarget = GetRandomPlayer(m_iDisguiseTeam, m_iDisguiseClass);

	if(!pTarget)
	{
		CBasePlayer *pTarget = GetRandomPlayer(m_iDisguiseTeam, 0);
	}

	if(pTarget)
	{
		m_pDisguiseTarget = pTarget;
		m_iDisguiseTarget = pTarget->entindex();
		m_iDisguiseMaxHealth = min(pTarget->pev->max_health, GetClassMaxHealth(m_iDisguiseClass));
		if(pTarget->m_iClass == m_iDisguiseClass)
			m_iDisguiseHealth = max(min(pTarget->pev->health, m_iDisguiseMaxHealth), m_iDisguiseMaxHealth*0.1);
		else
			m_iDisguiseHealth = m_iDisguiseMaxHealth * RANDOM_FLOAT(0.1, 1);
	}
	else
	{
		m_pDisguiseTarget = NULL;
		m_iDisguiseTarget = 0;
		m_iDisguiseMaxHealth = GetClassMaxHealth(m_iDisguiseClass);
		m_iDisguiseHealth = m_iDisguiseMaxHealth * RANDOM_FLOAT(0.1, 1.0);
	}

	Disguise_Weapon();

	m_iDisguiseMaxSpeed = min(GetClassMaxSpeed(m_iDisguiseClass), GetClassMaxSpeed(CLASS_SPY));

	//ALERT(at_console, "Disguise done.\n");

	/*MESSAGE_BEGIN(MSG_ONE, gmsgDisguise, NULL, pev);
	WRITE_BYTE(m_iDisguiseClass);
	WRITE_BYTE(m_iDisguiseTeam);
	MESSAGE_END();*/

	MESSAGE_BEGIN(MSG_ONE, gmsgDrawFX, NULL, pev);
	WRITE_BYTE(FX_DISGUISEHINT);
	WRITE_BYTE(1);//fadein
	MESSAGE_END();

	m_iDisguise = DISGUISE_YES;
	m_bDisguiseStart = false;

	ResetMaxSpeed();
}

void CBasePlayer::Disguise_Stop(void)
{
	if(m_iDisguise != DISGUISE_YES)
		return;

	//don't send msg to dead player 'cause we don't have player status hud
	if(IsAlive())
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgDrawFX, NULL, pev);
		WRITE_BYTE(FX_DISGUISEHINT);
		WRITE_BYTE(0);//fadeout
		MESSAGE_END();
	}

	m_iDisguise = DISGUISE_NO;
	m_bDisguiseStart = false;
	m_iDisguiseClass = 0;
	m_iDisguiseTeam = 0;
	ResetMaxSpeed();
}

int GetBuildMetal(int BuildClass)
{
	switch(BuildClass)
	{
	case BUILDABLE_SENTRY:return 130;
	case BUILDABLE_DISPENSER:return 100;
	case BUILDABLE_ENTRANCE:return 125;
	case BUILDABLE_EXIT:return 125;
	}
	return 0;
}

void CBasePlayer::Build_Start(int iBuildClass)
{
	if(GetBuildMetal(iBuildClass) > m_iMetal)//Low metal
		return;

	if(m_pBuildable[iBuildClass-1])//Already have
		return;

	m_iCarryBluePrint = iBuildClass;
	m_iBluePrintYaw = 0;

	MESSAGE_BEGIN(MSG_ONE, gmsgDrawFX, NULL, pev);
	WRITE_BYTE(FX_BLUEPRINT);
	WRITE_BYTE(iBuildClass);
	MESSAGE_END();
}

int CBasePlayer::AddPlayerMetal(int iMetalAdd)
{
	if(m_iClass != CLASS_ENGINEER)
		return 0;

	if(iMetalAdd == 0)
		return 0;

	int iOldMetal = m_iMetal;

	m_iMetal = max(0, min(m_iMetal + iMetalAdd, 200));

	SendAddMetal(m_iMetal - iOldMetal);

	return m_iMetal - iOldMetal;
}

BOOL UTIL_IsHullDefaultEx(Vector vecOrigin, float size, float height, edict_t *pEnt);
BOOL UTIL_IsHullInZone(edict_t *e, Vector vecMins, Vector vecMaxs);

BOOL CBasePlayer::Build_PreDeploy(Vector &vecSrc)
{
	if(!(pev->flags & FL_ONGROUND))
		return FALSE;

	float ground;
	TraceResult tr;
	Vector vecMins,vecMaxs;
	Vector vecSrc2;
	edict_t *pHit[9];

	edict_t *pEdictPlayer = edict();

	const static int vecDir[8][2] = {{1, 1}, {1, -1}, {-1, -1}, {-1, 1}, {1, 0}, {-1, 0}, {0, 1}, {0, -1}};

	ground = -9999;

	float height;
	if(m_iCarryBluePrint == BUILDABLE_SENTRY)
		height = 24;
	else if(m_iCarryBluePrint == BUILDABLE_DISPENSER)
		height = 26;
	else
		height = 6;

	for(int i = 0; i < 8; ++i)
	{
		vecSrc2 = vecSrc;
		vecSrc2.x += 16 * vecDir[i][0];
		vecSrc2.y += 16 * vecDir[i][1];
		UTIL_TraceLine(vecSrc2, vecSrc2 + Vector(0,0, -9999), dont_ignore_monsters, pEdictPlayer, &tr);
		if(tr.vecEndPos.z > ground)
			ground = tr.vecEndPos.z;
		pHit[i] = tr.pHit;
	}

	UTIL_TraceLine(vecSrc, vecSrc + Vector(0,0, -9999), dont_ignore_monsters, pEdictPlayer, &tr);
	if(tr.vecEndPos.z > ground)
		ground = tr.vecEndPos.z;
	pHit[8] = tr.pHit;

	for(int i = 0; i < 9; ++i)
	{
		if(!pHit[i])
			continue;
		CBaseEntity *pHitEntity = CBaseEntity::Instance(pHit[i]);
		if(FNullEnt(pHitEntity))
			continue;
		if(pHitEntity->IsPlayer())
		{
			CBasePlayer *pHitPlayer = (CBasePlayer *)pHitEntity;
			if(pHitPlayer->m_iTeam != m_iTeam)
				return FALSE;
		}
		if(pHitEntity->Classify() == CLASS_BUILDABLE)
			return FALSE;
	}

	if(vecSrc.z - (ground + height) > 64)
		return FALSE;

	//plane normal check
	if(tr.vecPlaneNormal.z < 0.866)
		return FALSE;

	vecSrc = tr.vecEndPos;
	vecSrc.z = ground;

	//no penetration for wall
	UTIL_TraceLine(GetGunPosition(), vecSrc, dont_ignore_monsters, pEdictPlayer, &tr);

	if(tr.flFraction != 1)
		return FALSE;

	//hull check
	if(!UTIL_IsHullDefaultEx(vecSrc + Vector(0,0,height), 16, height, pEdictPlayer))
		return FALSE;

	vecMins = vecSrc+Vector(-16, -16, -height);
	vecMaxs = vecSrc+Vector(16, 16, height);
	//nobuildable zone check
	for(int i = 0; i < g_pGameRules->m_NoBuildZone.Count(); ++i)
	{
		if(g_pGameRules->m_NoBuildZone[i]->v.team != 0 && g_pGameRules->m_NoBuildZone[i]->v.team != m_iTeam)
			continue;
		if(UTIL_IsHullInZone(g_pGameRules->m_NoBuildZone[i], vecMins, vecMaxs))
			return FALSE;
	}

	//just in case of stucking
	for(int i = 0; i < 4; ++i)
	{
		if(!m_pBuildable[i] || FNullEnt(m_pBuildable[i]))
			continue;
		Vector vecBuild = m_pBuildable[i]->Center();
		if(fabs(vecBuild.z - vecSrc.z) > 24)
			continue;
		vecBuild.z = vecSrc.z;
		if((vecBuild-vecSrc).Length() < 36)
			return FALSE;
	}
	return TRUE;
}

BOOL CBasePlayer::Build_PreUndeploy(void)
{
	if(m_iClass != CLASS_ENGINEER || m_iCarryBluePrint)
		return FALSE;

	if(!(pev->flags & FL_ONGROUND))
		return FALSE;

	TraceResult tr;
	Vector vecSrc, vecDst;

	UTIL_MakeVectors(pev->v_angle + pev->punchangle);

	vecSrc = GetGunPosition();

	vecDst = vecSrc + gpGlobals->v_forward * 128;

	UTIL_TraceLine(vecSrc, vecDst, dont_ignore_monsters, edict(), &tr);

	if(tr.flFraction == 1 || !tr.pHit)
		return FALSE;

	CBaseEntity *pEnt = CBaseEntity::Instance(tr.pHit);

	if(pEnt->Classify() != CLASS_BUILDABLE)
		return FALSE;

	Vector vecLength = (pEnt->Center() - vecSrc);
	if( vecLength.Length() > 64 )
		return FALSE;

	//is it my building? and it's not sapped?
	for(int i = 0; i < 4; ++i)
	{
		if(m_pBuildable[i])
		{
			if(pEnt->pev == m_pBuildable[i]->pev)
			{
				if(m_pBuildable[i]->m_pSapper)
					return FALSE;
				if((m_pBuildable[i]->m_iFlags & BUILD_UPGRADING) || (m_pBuildable[i]->m_iFlags & BUILD_BUILDING))
					return FALSE;

				m_pCarryBuild = (CBaseBuildable *)pEnt;
				return TRUE;
			}
		}
	}

	return FALSE;
}

void CBasePlayer::Build_Deploy(void)
{
	if(m_iCarryBluePrint < BUILDABLE_SENTRY || m_iCarryBluePrint > BUILDABLE_EXIT)
		return;

	Vector vecAngles = pev->v_angle + pev->punchangle;
	vecAngles.x = 0;

	UTIL_MakeVectors(vecAngles);
	Vector vecSrc = GetGunPosition() + gpGlobals->v_forward * 60;

	if(!Build_PreDeploy(vecSrc))
		return;

	int iBuildClass = m_iCarryBluePrint;

	CBaseBuildable *pBuild = NULL;

	vecAngles.y -= 90*m_iBluePrintYaw;

	if(m_pCarryBuild)
	{
		pBuild = m_pCarryBuild;
		pBuild->Rebuild( vecSrc, vecAngles );
		m_pCarryBuild = NULL;
	}
	else
	{
		if(iBuildClass == BUILDABLE_SENTRY)
			pBuild = CBuildSentry::CreateBuildable( vecSrc, vecAngles, this );

		else if(iBuildClass == BUILDABLE_DISPENSER)
			pBuild = CBuildDispenser::CreateBuildable( vecSrc, vecAngles, this );

		else if(iBuildClass == BUILDABLE_ENTRANCE)
			pBuild = CBuildTeleporter::CreateBuildable(TRUE, vecSrc, vecAngles, this );

		else if(iBuildClass == BUILDABLE_EXIT)
			pBuild = CBuildTeleporter::CreateBuildable(FALSE, vecSrc, vecAngles, this );

		if(pBuild)
		{
			AddPlayerMetal(-GetBuildMetal(iBuildClass));
		}
	}

	m_iCarryBluePrint = 0;
	m_iBluePrintYaw = 0;

	//tell client to hide the blueprint model
	MESSAGE_BEGIN(MSG_ONE, gmsgDrawFX, NULL, pev);
	WRITE_BYTE(FX_BLUEPRINT);
	WRITE_BYTE(0);
	MESSAGE_END();

	SelectLastItem();
}

void CBasePlayer::Build_Undeploy(void)
{
	if(!Build_PreUndeploy())
		return;

	if(!m_pCarryBuild)
		return;

	int iBuildClass = m_pCarryBuild->GetBuildClass();

	m_iCarryBluePrint = iBuildClass;
	m_iBluePrintYaw = 0;

	MESSAGE_BEGIN(MSG_ONE, gmsgDrawFX, NULL, pev);
	WRITE_BYTE(FX_BLUEPRINT);
	WRITE_BYTE(iBuildClass);
	MESSAGE_END();

	m_pCarryBuild->Undeploy();
	//switch to build pda
	SwitchSlotWeapon(WEAPON_SLOT_PDA);	
}

void CBasePlayer::Build_UpdateHUD(void)
{
	for(int i = 0; i < 4; ++i)
	{
		if(m_pBuildable[i])
			m_pBuildable[i]->UpdateHUD();
	}
}

void CBasePlayer::Build_DestroyAll(void)
{
	for(int i = 0; i < 4; ++i)
	{
		if(m_pBuildable[i])
		{
			m_pBuildable[i]->Killed(pev, -1);
			m_pBuildable[i] = NULL;
		}
	}
	MESSAGE_BEGIN(MSG_ONE, gmsgHUDBuild, NULL, pev);
	WRITE_BYTE(HUDBUILD_RESET);
	MESSAGE_END();
}

BOOL CBasePlayer::EatAmmoBox(float flGivePercent)
{
	if(flGivePercent <= 0)
		return FALSE;

	BOOL bAteShit = FALSE;

	if(m_iClass == CLASS_SPY && m_flCloakEnergy < 100) 
	{
		m_flCloakEnergy = min(m_flCloakEnergy+flGivePercent, 100);
		pev->armorvalue = m_flCloakEnergy;
		bAteShit = TRUE; 
	}

	if(m_iClass == CLASS_ENGINEER && m_iMetal < 200) 
	{
		AddPlayerMetal(flGivePercent*2);

		bAteShit = TRUE; 
	}

	CBasePlayerItem *pWeapon = NULL;
	char *pszAmmo = NULL;
	int iMaxAmmo = 0;
	for (int i = WEAPON_SLOT_PRIMARY; i <= WEAPON_SLOT_SECONDARY; i++)
	{
		pWeapon = m_rgpPlayerItems[i];
		if(pWeapon)
		{
			iMaxAmmo = pWeapon->iMaxAmmo1();
			pszAmmo = (char *)pWeapon->pszAmmo1();
			if(pszAmmo && iMaxAmmo)
			{
				if(GiveAmmo(max(iMaxAmmo*flGivePercent/100, 0), pszAmmo, iMaxAmmo) > 0)
					bAteShit = TRUE; 
			}
		}
	}
	return bAteShit;
}

BOOL CBasePlayer::EatAmmoBox(float flAmmoPercent, float flMetalPercent, float flCloakPercent)
{
	BOOL bAteShit = FALSE;

	if(m_iClass == CLASS_SPY && m_flCloakEnergy < 100 && flCloakPercent > 0) 
	{
		m_flCloakEnergy = min(m_flCloakEnergy+flCloakPercent, 100);
		pev->armorvalue = m_flCloakEnergy;
		bAteShit = TRUE;
	}

	if(m_iClass == CLASS_ENGINEER && m_iMetal < 200 && flMetalPercent > 0) 
	{
		AddPlayerMetal(flMetalPercent*2);

		bAteShit = TRUE;
	}

	if(flAmmoPercent > 0)
	{
		CBasePlayerItem *pWeapon = NULL;
		char *pszAmmo = NULL;
		int iMaxAmmo = 0;
		for (int i = WEAPON_SLOT_PRIMARY; i <= WEAPON_SLOT_SECONDARY; i++)
		{
			pWeapon = m_rgpPlayerItems[i];
			if(pWeapon)
			{
				iMaxAmmo = pWeapon->iMaxAmmo1();
				pszAmmo = (char *)pWeapon->pszAmmo1();
				if(pszAmmo && iMaxAmmo)
				{
					if(GiveAmmo(max(iMaxAmmo*flAmmoPercent/100, 0), pszAmmo, iMaxAmmo) > 0)
						bAteShit = TRUE; 
				}
			}
		}
	}
	return bAteShit;
}

void CBasePlayer::PlayerDisconnect(void)
{
	Build_DestroyAll();
	ClearSticky();
	ClearDominates();
	ClearEffects();
}

void CBasePlayer::KnockBack(Vector vecDir, float flForce)
{
	float flRealForce = flForce * GetKnockBack();

	if(flRealForce == 0)
		return;

	pev->velocity = pev->velocity + vecDir.Normalize() * flRealForce;
}

void CBasePlayer::Build_Demolish(int iBuildClass)
{
	if(!m_pBuildable[iBuildClass-1])//Don't have one
		return;

	m_pBuildable[iBuildClass-1]->Killed(pev, 0);

	SelectLastItem();
}

void CBasePlayer::SendStatsInfo(int iType)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgStatsInfo, NULL, pev);
	WRITE_BYTE(iType);
	switch(iType)
	{
	case STATS_DEMOLISH:
		WRITE_SHORT(m_Stats.iDemolish);
		break;
	case STATS_CAPTURE:
		WRITE_SHORT(m_Stats.iCapture);
		break;
	case STATS_DEFENCE:
		WRITE_SHORT(m_Stats.iDefence);
		break;
	case STATS_DOMINATE:
		WRITE_SHORT(m_Stats.iDominate);
		break;
	case STATS_REVENGE:
		WRITE_SHORT(m_Stats.iRevenge);
		break;
	case STATS_UBERCHARGE:
		WRITE_SHORT(m_Stats.iUbercharge);
		break;
	case STATS_HEADSHOT:
		WRITE_SHORT(m_Stats.iHeadshot);
		break;
	case STATS_TELEPORT:
		WRITE_SHORT(m_Stats.iTeleport);
		break;
	case STATS_HEALING:
		WRITE_SHORT(m_Stats.iHealing);
		break;
	case STATS_BACKSTAB:
		WRITE_SHORT(m_Stats.iBackstab);
		break;
	case STATS_BONUS:
		WRITE_SHORT(m_Stats.iBonus);
		break;
	case STATS_KILL:
		WRITE_SHORT(m_Stats.iKill);
		break;
	case STATS_DEATH:
		WRITE_SHORT(m_Stats.iDeath);
		break;
	case STATS_ASSIST:
		WRITE_SHORT(m_Stats.iAssist);
		break;
	case STATS_CLEAR:
		WRITE_SHORT(0);
		break;
	}
	MESSAGE_END();
}

void CBasePlayer::SendScoreInfo(void)
{
	MESSAGE_BEGIN(MSG_ALL, gmsgScoreInfo);
	WRITE_BYTE(ENTINDEX(edict()));
	WRITE_SHORT((int)pev->frags);
	WRITE_SHORT(m_Stats.iDeath);
	WRITE_BYTE(m_iDominates);
	MESSAGE_END();
}

void CBasePlayer::DmgRecord_Add(CBasePlayer *pAttacker, int iDamage)
{
	int count = m_DmgRecord.Count();
	if(count)
	{
		int i;
		for(i = 0; i < count; ++i)
		{
			if(gpGlobals->time - m_DmgRecord[i].flTime < 30)
				break;
		}
		if(i > 0)
		{
			m_DmgRecord.RemoveMultiple(0, i);
		}
	}

	dmgrecord_t &dmg = m_DmgRecord[m_DmgRecord.AddToTail()];

	dmg.pevAttacker = pAttacker->pev;
	dmg.iDamage = iDamage;
	dmg.flTime = gpGlobals->time;
}

int CBasePlayer::DmgRecord_Get(CBasePlayer *pAttacker, float flBefore)
{
	int iCount = 0;
	for(int i = m_DmgRecord.Count() - 1; i >= 0; --i)
	{
		if(gpGlobals->time - m_DmgRecord[i].flTime > 30)
			break;

		iCount += m_DmgRecord[i].iDamage;
	}

	return iCount;
}

void CBasePlayer::DmgRecord_Clear(void)
{
	m_DmgRecord.RemoveAll();
}

int CBasePlayer::GetCriticalHit(void)
{
	return 0;
}

bool CBasePlayer::IsObservingPlayer(CBasePlayer *pTarget)
{
	if (!pTarget)
		return false;

	if (pev->flags == FL_DORMANT)
		return false;

	if (FNullEnt(pTarget))
		return false;

	if (pev->iuser1 == OBS_IN_EYE && pev->iuser2 == ENTINDEX(pTarget->edict()))
		return true;

	return false;
}

void CBasePlayer::SetObserverAutoDirector(bool bState)
{
	m_bObserverAutoDirector = bState;
}

bool CBasePlayer::CanSwitchObserverModes(void)
{
	return m_canSwitchObserverModes;
}

void CBasePlayer::SendSpecHealth(bool bShowEntIndex, entvars_t *pevInflictor)
{
	MESSAGE_BEGIN(MSG_SPEC, SVC_DIRECTOR);
	WRITE_BYTE(9);
	WRITE_BYTE(DRC_CMD_EVENT);
	WRITE_SHORT(ENTINDEX(edict()));
	WRITE_SHORT(ENTINDEX(ENT(pevInflictor)));
	WRITE_LONG(5);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_SPEC, gmsgHLTV);
	WRITE_BYTE(ENTINDEX(edict()));
	WRITE_BYTE((int)(pev->health ? pev->health : 0) | 128);
	MESSAGE_END();

	for (int i = 1; i < gpGlobals->maxClients; i++)
	{
		CBasePlayer *temp = (CBasePlayer *)UTIL_PlayerByIndex(i);

		if (!temp)
			continue;

		if (temp->m_hObserverTarget == this)
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgSpecHealth, NULL, temp->pev);
			WRITE_BYTE(pev->health ? pev->health : 0);
			MESSAGE_END();
		}
	}
}

void CBasePlayer::UpdateDominate(void)
{
	int count = 0;
	for(int i = 1; i < gpGlobals->maxClients; ++i)
	{
		CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex(i);
		if(m_Dominate[i] >= 4 && pPlayer && !pPlayer->IsDormant())
		{
			if(pPlayer->m_iTeam == 3 - m_iTeam)
				count ++;
		}
	}
	m_iDominates = count;
}

void CBasePlayer::ClearDominates(void)
{
	int myindex = entindex();
	for(int i = 1; i <= gpGlobals->maxClients; ++i)
	{
		CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex(i);
		if(pPlayer && !pPlayer->IsDormant() && pPlayer->m_Dominate[myindex] >= 4)
		{
			pPlayer->m_Dominate[myindex] = 0;
			pPlayer->UpdateDominate();
			pPlayer->SendScoreInfo();
		}
	}
}

void CBasePlayer::FlushHitDamage(void)
{
	for(int i = 0; i < m_HitDamage.Count(); ++i)
	{
		CBaseEntity *pEntity = CBaseEntity::Instance(m_HitDamage[i].pevAttacker);
		if(pEntity && pEntity->IsPlayer() && !pEntity->IsDormant())
		{
			CBasePlayer *pPlayer = (CBasePlayer *)pEntity;
			pPlayer->SendHitDamage(pev, m_HitDamage[i].iDamage, m_HitDamage[i].iCrit);
		}
	}
	m_HitDamage.RemoveAll();
}

void CBasePlayer::AddHitDamage(entvars_t *pevAttacker, int iDamage, int iCrit)
{
	hitdamage_t *dmg = NULL;
	for(int i = 0; i < m_HitDamage.Count(); ++i)
	{
		if(m_HitDamage[i].pevAttacker == pevAttacker)
		{
			dmg = &m_HitDamage[i];
			break;
		}
	}
	if(!dmg)//found an exist one
	{
		dmg = &m_HitDamage[m_HitDamage.AddToTail()];
		dmg->pevAttacker = pevAttacker;
		dmg->iDamage = iDamage;
		dmg->iCrit = iCrit;
	}
	else
	{
		dmg->iDamage += iDamage;
		if(iCrit > dmg->iCrit)
			dmg->iCrit = iCrit;
	}
}

void CBasePlayer::ClearEffects(void)
{
	MESSAGE_BEGIN(MSG_BROADCAST, gmsgDrawFX);
	WRITE_BYTE(FX_KILLTRAIL);
	WRITE_SHORT(entindex());
	MESSAGE_END();
}