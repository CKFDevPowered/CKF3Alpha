#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "gamerules.h"
#include "skill.h"
#include "game.h"
#include "items.h"
#include "hltv.h"
#include "client.h"
#include "mapinfo.h"
#include "shake.h"
#include "trains.h"
#include "vehicle.h"
#include "trigger.h"
#include "buildable.h"
#include "metarender.h"

extern DLL_GLOBAL CHalfLifeMultiplay *g_pGameRules;
extern DLL_GLOBAL BOOL g_fGameOver;
extern float g_flTimeLimit;
extern float g_flResetTime;

extern int gmsgDeathMsg;
extern int gmsgBuildDeath;
extern int gmsgScoreInfo;
extern int gmsgMOTD;
extern int gmsgServerName;
extern int gmsgSendAudio;
extern int gmsgBombPickup;
extern int gmsgTeamScore;
extern int gmsgTimeLimit;
extern int gmsgRevenge;
extern int gmsgDominate;
extern int gmsgMapObject;
extern int gmsgFog;
extern int gmsgHLTV;

#define ITEM_RESPAWN_TIME 10
#define WEAPON_RESPAWN_TIME 10
#define AMMO_RESPAWN_TIME 10

class CCStrikeGameMgrHelper : public IVoiceGameMgrHelper
{
public:
	virtual bool CanPlayerHearPlayer(CBasePlayer *pListener, CBasePlayer *pTalker)
	{
		if (pListener->m_iTeam != pTalker->m_iTeam)
			return false;

		BOOL pListenerAlive = pListener->IsAlive();
		BOOL pTalkerAlive = pTalker->IsAlive();

		if (pListener->IsObserver())
			return true;

		if (pListenerAlive)
		{
			if (!pTalkerAlive)
				return false;
		}
		else if (pTalkerAlive)
			return true;

		return pListenerAlive == pTalkerAlive;
	}
};

CCStrikeGameMgrHelper g_GameMgrHelper;

void Broadcast(const char *sentence)
{
	char text[256];

	if (!sentence)
		return;

	strcpy(text, "%!MRAD_");
	strcat(text, UTIL_VarArgs("%s", sentence));

	MESSAGE_BEGIN(MSG_BROADCAST, gmsgSendAudio);
	WRITE_BYTE(0);
	WRITE_STRING(text);
	WRITE_SHORT(100);
	MESSAGE_END();
}

char *GetTeam(int team)
{
	switch (team)
	{
		case TEAM_TERRORIST: return "TERRORIST";
		case TEAM_CT: return "CT";
		case TEAM_SPECTATOR: return "SPECTATOR";
	}

	return "";
}

#define Target_Bombed 1
#define VIP_Escaped 2
#define VIP_Assassinated 3
#define Terrorists_Escaped 4
#define CTs_PreventEscape 5
#define Escaping_Terrorists_Neutralized 6
#define Bomb_Defused 7
#define CTs_Win 8
#define Terrorists_Win 9
#define Round_Draw 10
#define All_Hostages_Rescued 11
#define Target_Saved 12
#define Hostages_Not_Rescued 13
#define Terrorists_Not_Escaped 14
#define VIP_Not_Escaped 15

void EndRoundMessage(const char *message, int type)
{
	int iWinTeam = 0;
	char *teamname = NULL;
	BOOL bTeamWins = TRUE;

	UTIL_ClientPrintAll(HUD_PRINTCENTER, message);

	switch (type)
	{
		case Target_Bombed:
		case VIP_Assassinated:
		case Terrorists_Escaped:
		case Terrorists_Win:
		case Hostages_Not_Rescued:
		case VIP_Not_Escaped: teamname = GetTeam(TEAM_TERRORIST); iWinTeam = TEAM_RED; break;
		case VIP_Escaped:
		case CTs_PreventEscape:
		case Escaping_Terrorists_Neutralized:
		case Bomb_Defused:
		case CTs_Win:
		case All_Hostages_Rescued:
		case Target_Saved:
		case Terrorists_Not_Escaped: teamname = GetTeam(TEAM_CT); iWinTeam = TEAM_BLU; break;
		default: bTeamWins = FALSE; break;
	}

	if (g_pGameRules)
	{
		if (bTeamWins == TRUE)
			UTIL_LogPrintf("Team \"%s\" triggered \"%s\" (CT \"%i\") (T \"%i\")\n", teamname, &message[1], g_pGameRules->m_iNumCTWins, g_pGameRules->m_iNumTerroristWins);
		else
			UTIL_LogPrintf("World triggered \"%s\" (CT \"%i\") (T \"%i\")\n", &message[1], g_pGameRules->m_iNumCTWins, g_pGameRules->m_iNumTerroristWins);
	}

	UTIL_LogPrintf("World triggered \"Round_End\"\n");
}

CHalfLifeMultiplay::CHalfLifeMultiplay(void)
{
	int i;

	m_VoiceGameMgr.Init(&g_GameMgrHelper, gpGlobals->maxClients);
	RefreshSkillData();

	m_flIntermissionStartTime = 0;
	m_flIntermissionEndTime = 0;
	m_iRoundWinStatus = 0;
	m_iNumTerroristWins = m_iNumCTWins = 0;
	m_pVIP = NULL;
	m_iNumTerrorist = m_iNumCT = 0;
	m_iNumSpawnableTerrorist = m_iNumSpawnableCT = 0;
	m_iMapHasIntroCamera = 2;
	g_fGameOver = FALSE;
	m_iNumConsecutiveCTLoses = 0;
	m_iNumConsecutiveTerroristLoses = 0;
	m_bLevelInitialized = FALSE;
	m_tmNextPeriodicThink = 0;
	m_flRoundTimer = gpGlobals->time;
	m_iRoundTimeMax = 0;
	m_flAnnounceRoundTime = 0;
	m_bFirstConnected = FALSE;
	m_bCompleteReset = FALSE;
	m_iUnBalancedRounds = 0;
	m_bRoundTerminating = FALSE;
	m_iLimitTeam = 0;
	//Round init
	m_iMaxRounds = (int)CVAR_GET_FLOAT("mp_maxrounds");
	//CP init
	m_iRedLocal = m_iBluLocal = 0;
	m_ControlPoints.RemoveAll();
	//NoBuildZone init
	m_NoBuildZone.RemoveAll();
	m_ShadowManager.RemoveAll();
	m_iSetupCondition = ROUND_NORMAL;
	m_iEndAction = END_DRAW;
	m_iRedDominatedAction = END_RED_WIN;
	m_iBluDominatedAction = END_BLU_WIN;
	m_iWaitTime = 30;
	m_iFreezeTime = 3;
	m_iSetupTime = 60;
	m_iRoundTime = 300;
	m_iEndTime = 15;	
	m_bFreezePeriod = false;
	m_flFreezeTimer = 0;
	m_iRespawnDisabled = 0;
	m_bMapHasControlPoint = false;
	g_SkyCamera.enable = false;

	if (m_iMaxRounds < 0)
	{
		m_iMaxRounds = 0;
		CVAR_SET_FLOAT("mp_maxrounds", 0);
	}

	m_iTotalRoundsPlayed = 0;
	m_iWinLimit = (int)CVAR_GET_FLOAT("mp_winlimit");

	if (m_iWinLimit < 0)
	{
		m_iWinLimit = 0;
		CVAR_SET_FLOAT("mp_winlimit", 0);
	}

	for (i = 0; i < MAX_MAPS; i++)//memset is better
		m_rgiVotingMapCount[i] = 0;

	m_iAllowSpectators = allow_spectators.value;

	CVAR_SET_FLOAT("cl_himodels", 0);

	if (IS_DEDICATED_SERVER())
	{
		const char *servercfgfile = CVAR_GET_STRING("servercfgfile");

		if (servercfgfile && *servercfgfile)
		{
			char szCommand[256];
			ALERT(at_console, "Executing dedicated server config file\n");
			sprintf(szCommand, "exec %s\n", servercfgfile);
			SERVER_COMMAND(szCommand);
		}
	}
	else
	{
		const char *servercfgfile = CVAR_GET_STRING("lservercfgfile");

		if (servercfgfile && *servercfgfile)
		{
			char szCommand[256];
			ALERT(at_console, "Executing listen server config file\n");
			sprintf(szCommand, "exec %s\n", servercfgfile);
			SERVER_COMMAND(szCommand);
		}
	}

	CVAR_SET_FLOAT("sv_restart", 3);
}

void CHalfLifeMultiplay::ReadMultiplayCvars(void)
{
	//Read the timer cvar
	m_iWaitTime = (int)CVAR_GET_FLOAT("mp_waittime");
	m_iFreezeTime = (int)CVAR_GET_FLOAT("mp_freezetime");
	m_iSetupTime = (int)CVAR_GET_FLOAT("mp_setuptime");
	m_iRoundTime = (int)CVAR_GET_FLOAT("mp_roundtime");
	m_iEndTime = (int)CVAR_GET_FLOAT("mp_endtime");

	m_iLimitTeam = (int)CVAR_GET_FLOAT("mp_limitteams");
	m_fRedWaveTime = CVAR_GET_FLOAT("mp_redwavetime");
	m_fBluWaveTime = CVAR_GET_FLOAT("mp_bluwavetime");
	m_fRedMaxRespawn = CVAR_GET_FLOAT("mp_redmaxrespawn");
	m_fRedMinRespawn = CVAR_GET_FLOAT("mp_redminrespawn");
	m_fBluMaxRespawn = CVAR_GET_FLOAT("mp_blumaxrespawn");
	m_fBluMinRespawn = CVAR_GET_FLOAT("mp_bluminrespawn");
	m_fRespawnPlayerFactor = CVAR_GET_FLOAT("mp_respawn_playerfactor");
	m_iRespawnPlayerNum = (int)CVAR_GET_FLOAT("mp_respawn_playernum");
	m_iRespawnDisabled = (int)CVAR_GET_FLOAT("mp_disable_respawn");

	//Bound the timer cvar

	if (m_iFreezeTime > 60)
	{
		CVAR_SET_FLOAT("mp_freezetime", 60);
		m_iFreezeTime = 60;
	}
	else if (m_iFreezeTime < 0)
	{
		CVAR_SET_FLOAT("mp_freezetime", 0);
		m_iFreezeTime = 0;
	}

	if (m_iSetupTime > 300)
	{
		CVAR_SET_FLOAT("mp_setuptime", 300);
		m_iSetupTime = 300;
	}
	else if (m_iSetupTime < 0)
	{
		CVAR_SET_FLOAT("mp_setuptime", 0);
		m_iSetupTime = 0;
	}

	if (m_iRoundTime > 32767)//540
	{
		CVAR_SET_FLOAT("mp_roundtime", 32767);
		m_iRoundTime = 32767;
	}
	else if (m_iRoundTime < 30)
	{
		CVAR_SET_FLOAT("mp_roundtime", 30);
		m_iRoundTime = 30;
	}

	if (m_iEndTime > 60)
	{
		CVAR_SET_FLOAT("mp_endtime", 60);
		m_iEndTime = 60;
	}
	else if (m_iEndTime < 0)
	{
		CVAR_SET_FLOAT("mp_endtime", 0);
		m_iEndTime = 0;
	}

	if (m_iWaitTime > 60)
	{
		CVAR_SET_FLOAT("mp_waittime", 60);
		m_iEndTime = 60;
	}
	else if (m_iWaitTime < 0)
	{
		CVAR_SET_FLOAT("mp_waittime", 0);
		m_iEndTime = 0;
	}

	if (m_iLimitTeam > 20)
	{
		CVAR_SET_FLOAT("mp_limitteams", 20);
		m_iLimitTeam = 20;
	}
	else if (m_iLimitTeam <= 0)
	{
		CVAR_SET_FLOAT("mp_limitteams", 0);
		m_iLimitTeam = 20;
	}
}

void CHalfLifeMultiplay::RefreshSkillData(void)
{
	CGameRules::RefreshSkillData();

	gSkillData.suitchargerCapacity = 30;
	gSkillData.plrDmgCrowbar = 25;
	gSkillData.plrDmg9MM = 12;
	gSkillData.plrDmg357 = 40;
	gSkillData.plrDmgMP5 = 12;
	gSkillData.plrDmgM203Grenade = 100;
	gSkillData.plrDmgBuckshot = 20;
	gSkillData.plrDmgCrossbowClient = 20;
	gSkillData.plrDmgRPG = 120;
	gSkillData.plrDmgEgonWide = 20;
	gSkillData.plrDmgEgonNarrow = 10;
	gSkillData.plrDmgHandGrenade = 100;
	gSkillData.plrDmgSatchel = 120;
	gSkillData.plrDmgTripmine = 150;
	gSkillData.plrDmgHornet = 10;
}

void CHalfLifeMultiplay::RemoveGuns(void)
{
	CBaseEntity *pEntity = NULL;

	while ((pEntity = UTIL_FindEntityByClassname(pEntity, "weaponbox")) != NULL)
		UTIL_Remove(pEntity);
}

extern unsigned short m_usResetDecals;

void CHalfLifeMultiplay::CleanUpMap(void)//hz: round restart
{
	CBaseEntity *toRestart = UTIL_FindEntityByClassname(NULL, "light");

	while (toRestart)
	{
		toRestart->Restart();
		toRestart = UTIL_FindEntityByClassname(toRestart, "light");
	}

	toRestart = UTIL_FindEntityByClassname(NULL, "trigger_multiple");

	while (toRestart)
	{
		toRestart->Restart();
		toRestart = UTIL_FindEntityByClassname(toRestart, "trigger_multiple");
	}

	toRestart = UTIL_FindEntityByClassname(NULL, "func_wall_toggle");

	while (toRestart)
	{
		toRestart->Restart();
		toRestart = UTIL_FindEntityByClassname(toRestart, "func_wall_toggle");
	}

	toRestart = UTIL_FindEntityByClassname(NULL, "func_breakable");

	while (toRestart)
	{
		toRestart->Restart();
		toRestart = UTIL_FindEntityByClassname(toRestart, "func_breakable");
	}

	toRestart = UTIL_FindEntityByClassname(NULL, "func_pushable");

	while (toRestart)
	{
		toRestart->Restart();
		toRestart = UTIL_FindEntityByClassname(toRestart, "func_pushable");
	}

	toRestart = UTIL_FindEntityByClassname(NULL, "func_tank");

	while (toRestart)
	{
		toRestart->Restart();
		toRestart = UTIL_FindEntityByClassname(toRestart, "func_tank");
	}

	toRestart = UTIL_FindEntityByClassname(NULL, "func_tanklaser");

	while (toRestart)
	{
		toRestart->Restart();
		toRestart = UTIL_FindEntityByClassname(toRestart, "func_tanklaser");
	}

	toRestart = UTIL_FindEntityByClassname(NULL, "func_door");

	while (toRestart)
	{
		toRestart->Restart();
		toRestart = UTIL_FindEntityByClassname(toRestart, "func_door");
	}

	toRestart = UTIL_FindEntityByClassname(NULL, "func_water");

	while (toRestart)
	{
		toRestart->Restart();
		toRestart = UTIL_FindEntityByClassname(toRestart, "func_water");
	}

	toRestart = UTIL_FindEntityByClassname(NULL, "func_door_rotating");

	while (toRestart)
	{
		toRestart->Restart();
		toRestart = UTIL_FindEntityByClassname(toRestart, "func_door_rotating");
	}

	toRestart = UTIL_FindEntityByClassname(NULL, "func_tracktrain");

	while (toRestart)
	{
		toRestart->Restart();
		toRestart = UTIL_FindEntityByClassname(toRestart, "func_tracktrain");
	}

	toRestart = UTIL_FindEntityByClassname(NULL, "func_vehicle");

	while (toRestart)
	{
		toRestart->Restart();
		toRestart = UTIL_FindEntityByClassname(toRestart, "func_vehicle");
	}

	toRestart = UTIL_FindEntityByClassname(NULL, "func_train");

	while (toRestart)
	{
		toRestart->Restart();
		toRestart = UTIL_FindEntityByClassname(toRestart, "func_train");
	}

	toRestart = UTIL_FindEntityByClassname(NULL, "item_healthbox");

	while (toRestart)
	{
		toRestart->Restart();
		toRestart = UTIL_FindEntityByClassname(toRestart, "item_healthbox");
	}

	toRestart = UTIL_FindEntityByClassname(NULL, "item_ammobox");

	while (toRestart)
	{
		toRestart->Restart();
		toRestart = UTIL_FindEntityByClassname(toRestart, "item_ammobox");
	}

	toRestart = UTIL_FindEntityByClassname(NULL, "ambient_generic");

	while (toRestart)
	{
		toRestart->Restart();
		toRestart = UTIL_FindEntityByClassname(toRestart, "ambient_generic");
	}

	toRestart = UTIL_FindEntityByClassname(NULL, "env_sprite");

	while (toRestart)
	{
		toRestart->Restart();
		toRestart = UTIL_FindEntityByClassname(toRestart, "env_sprite");
	}

	//hz new entity
	toRestart = UTIL_FindEntityByClassname(NULL, "info_player_start");

	while (toRestart)
	{
		toRestart->Restart();
		toRestart = UTIL_FindEntityByClassname(toRestart, "info_player_start");
	}

	toRestart = UTIL_FindEntityByClassname(NULL, "info_player_deathmatch");

	while (toRestart)
	{
		toRestart->Restart();
		toRestart = UTIL_FindEntityByClassname(toRestart, "info_player_deathmatch");
	}

	CBaseEntity *toRemove;

	toRemove = UTIL_FindEntityByClassname(NULL, "pj_grenade");
	while (toRemove)
	{
		UTIL_Remove(toRemove);
		toRemove = UTIL_FindEntityByClassname(toRemove, "pj_grenade");
	}

	toRemove = UTIL_FindEntityByClassname(NULL, "pj_sticky");
	while (toRemove)
	{
		UTIL_Remove(toRemove);
		toRemove = UTIL_FindEntityByClassname(toRemove, "pj_sticky");
	}

	toRemove = UTIL_FindEntityByClassname(NULL, "pj_rocket");
	while (toRemove)
	{
		UTIL_Remove(toRemove);
		toRemove = UTIL_FindEntityByClassname(toRemove, "pj_rocket");
	}

	toRemove = UTIL_FindEntityByClassname(NULL, "pj_senrocket");
	while (toRemove)
	{
		UTIL_Remove(toRemove);
		toRemove = UTIL_FindEntityByClassname(toRemove, "pj_senrocket");
	}

	RemoveGuns();
	PLAYBACK_EVENT(FEV_GLOBAL | FEV_RELIABLE, 0, m_usResetDecals);

	MESSAGE_BEGIN(MSG_BROADCAST, gmsgDrawFX);
	WRITE_BYTE(FX_KILLALLTRAIL);
	MESSAGE_END();
}

void CHalfLifeMultiplay::CheckWinConditions(void)
{
	if (m_iRoundWinStatus)
		return;

	int NumDeadCT, NumDeadTerrorist, NumAliveTerrorist, NumAliveCT;
	BOOL bNeededPlayers = FALSE;
	InitializePlayerCounts(NumAliveTerrorist, NumAliveCT, NumDeadTerrorist, NumDeadCT);

	if (NeededPlayersCheck(bNeededPlayers))
		return;

	if (CPRoundEndCheck(bNeededPlayers))
		return;
}

void CHalfLifeMultiplay::TerminateRound(float tmDelay, int iWinStatus)
{
	g_pGameRules->m_iRoundWinStatus = iWinStatus;
	g_pGameRules->m_bRoundTerminating = TRUE;
	g_pGameRules->SetRoundStatus(ROUND_END, tmDelay);
	g_pGameRules->SyncRoundTimer();

	if(iWinStatus == WINSTATUS_TERRORIST)
	{
		FireTargets("game_round_redwin", g_pWorld, g_pWorld, USE_TOGGLE, 0);
	}
	else if(iWinStatus == WINSTATUS_CT)
	{
		FireTargets("game_round_bluwin", g_pWorld, g_pWorld, USE_TOGGLE, 0);
	}
	else
	{
		FireTargets("game_round_draw", g_pWorld, g_pWorld, USE_TOGGLE, 0);
	}

	char szVictoryMP3[128];
	char szFailureMP3[128];
	char szStalemateMP3[128];

	sprintf(szVictoryMP3, "mp3 play sound/CKF_III/ano/your_team_won.mp3\n");
	sprintf(szFailureMP3, "mp3 play sound/CKF_III/ano/your_team_lost.mp3\n");
	sprintf(szStalemateMP3, "mp3 play sound/CKF_III/ano/your_team_stalemate.mp3\n");

	bool bStripWeapon;
	CBaseEntity *pEntity = NULL;
	CBasePlayer *pPlayer = NULL;

	while ((pEntity = UTIL_FindEntityByClassname(pEntity, "player")) != NULL)
	{
		if (FNullEnt(pEntity->edict()))
			break;

		pPlayer = GetClassPtr((CBasePlayer *)pEntity->pev);

		if (pEntity->pev->flags == FL_DORMANT)
			continue;

		bStripWeapon = false;

		if(iWinStatus == WINSTATUS_DRAW)
		{
			CLIENT_COMMAND(pPlayer->edict(), szStalemateMP3);
			bStripWeapon = true;
		}
		else
		{
			if(iWinStatus == pPlayer->m_iTeam)
			{
				CLIENT_COMMAND(pPlayer->edict(), szVictoryMP3);
				//add critboost here
				pPlayer->CritBoost_Add(tmDelay + 1.0f);
			}
			else
			{
				CLIENT_COMMAND(pPlayer->edict(), szFailureMP3);
				bStripWeapon = true;
			}
		}
		if(bStripWeapon)
		{
			pPlayer->m_bAllowAttack = false;
			pPlayer->RemoveAllItems(FALSE);
			pPlayer->ResetMaxSpeed();
			pPlayer->m_iFOV = pPlayer->m_iDefaultFOV;

			if(pPlayer->m_iCarryBluePrint)
			{
				//tell client to hide the blueprint model
				MESSAGE_BEGIN(MSG_ONE, gmsgDrawFX, NULL, pPlayer->pev);
				WRITE_BYTE(FX_BLUEPRINT);
				WRITE_BYTE(0);
				MESSAGE_END();
			}
		}
	}
}

void CHalfLifeMultiplay::InitializePlayerCounts(int &NumAliveTerrorist, int &NumAliveCT, int &NumDeadTerrorist, int &NumDeadCT)
{
	NumAliveTerrorist = NumAliveCT = NumDeadCT = NumDeadTerrorist = 0;
	m_iNumTerrorist = m_iNumCT = m_iNumSpawnableTerrorist = m_iNumSpawnableCT = 0;

	CBaseEntity *pEntity = NULL;

	while ((pEntity = UTIL_FindEntityByClassname(pEntity, "player")) != NULL)
	{
		if (FNullEnt(pEntity->edict()))
			break;

		CBasePlayer *pPlayer = GetClassPtr((CBasePlayer *)pEntity->pev);

		if (pEntity->pev->flags == FL_DORMANT)
			continue;

		switch (pPlayer->m_iTeam)
		{
			case TEAM_CT:
			{
				m_iNumCT++;

				if (pPlayer->m_iJoiningState == JOINED)
					m_iNumSpawnableCT++;

				if (pPlayer->pev->deadflag != DEAD_NO)
					NumDeadCT++;
				else
					NumAliveCT++;

				break;
			}

			case TEAM_TERRORIST:
			{
				m_iNumTerrorist++;

				if (pPlayer->m_iJoiningState == JOINED)
					m_iNumSpawnableTerrorist++;

				if (pPlayer->pev->deadflag != DEAD_NO)
					NumDeadTerrorist++;
				else
					NumAliveTerrorist++;

				break;
			}
		}
	}
}

BOOL CHalfLifeMultiplay::NeededPlayersCheck(BOOL &bNeededPlayers)
{
	if (!m_iNumSpawnableTerrorist || !m_iNumSpawnableCT)
	{
		UTIL_ClientPrintAll(HUD_PRINTCONSOLE, "#CKF3_Game_Scoring");
		bNeededPlayers = TRUE;
	}

	/*if (!m_bFirstConnected && m_iNumSpawnableTerrorist && m_iNumSpawnableCT)
	{
		UTIL_LogPrintf("World triggered \"Game_Commencing\"\n");
		m_bCompleteReset = TRUE;

		//EndRoundMessage("#Game_Commencing", Round_Draw);
		//TerminateRound(1, WINSTATUS_DRAW);
		RestartRound();
		m_bFirstConnected = TRUE;
		return TRUE;
	}*/

	return FALSE;
}

BOOL CHalfLifeMultiplay::TeamExterminationCheck(int NumAliveTerrorist, int NumAliveCT, int NumDeadTerrorist, int NumDeadCT, BOOL bNeededPlayers)
{
	if (m_iNumCT > 0 && m_iNumSpawnableCT > 0 && m_iNumTerrorist > 0 && m_iNumSpawnableTerrorist > 0)
	{
		if (!NumAliveTerrorist && NumDeadTerrorist)
		{
			BOOL nowin = FALSE;
			CGrenade *pGrenade = NULL;

			if (!nowin)
			{
				Broadcast("ctwin");

				if (!bNeededPlayers)
				{
					m_iNumCTWins++;
					UpdateTeamScores();
				}

				EndRoundMessage("#CTs_Win", CTs_Win);
				TerminateRound(m_iEndTime, WINSTATUS_CT);
				return TRUE;
			}
		}
		else if (!NumAliveCT && NumDeadCT)
		{
			Broadcast("terwin");

			if (!bNeededPlayers)
			{
				m_iNumTerroristWins++;
				UpdateTeamScores();
			}

			EndRoundMessage("#Terrorists_Win", Terrorists_Win);
			TerminateRound(m_iEndTime, WINSTATUS_TERRORIST);
			return TRUE;
		}
	}
	else if (!NumAliveCT && !NumAliveTerrorist)
	{
		EndRoundMessage("#Round_Draw", Round_Draw);
		Broadcast("rounddraw");
		TerminateRound(m_iEndTime, WINSTATUS_DRAW);
		return TRUE;
	}

	return FALSE;
}

void CHalfLifeMultiplay::BalanceTeams(void)
{
	int iTeamToSwap, iNumToSwap;

	if (m_iNumCT > m_iNumTerrorist)
	{
		iTeamToSwap = TEAM_CT;
		iNumToSwap = (m_iNumCT - m_iNumTerrorist) / 2;
	}
	else if (m_iNumTerrorist > m_iNumCT)
	{
		iTeamToSwap = TEAM_TERRORIST;
		iNumToSwap = (m_iNumTerrorist - m_iNumCT) / 2;
	}
	else
	{
		return;
	}

	if (iNumToSwap > 4)
		iNumToSwap = 4;

	int iHighestUserID = 0;
	CBasePlayer *toSwap = NULL;
	CBaseEntity *pEntity = NULL;

	for (int i = 1; i <= iNumToSwap; i++)
	{
		iHighestUserID = 0;
		toSwap = NULL;

		while ((pEntity = UTIL_FindEntityByClassname(pEntity, "player")) != NULL)
		{
			if (FNullEnt(pEntity->edict()))
				break;

			if (pEntity->pev->flags == FL_DORMANT)
				continue;

			CBasePlayer *pPlayer = GetClassPtr((CBasePlayer *)pEntity->pev);

			if (pPlayer->m_iTeam != iTeamToSwap || GETPLAYERUSERID(ENT(pPlayer->pev)) <= iHighestUserID || m_pVIP == pPlayer)
				continue;

			iHighestUserID = GETPLAYERUSERID(ENT(pPlayer->pev));
			toSwap = pPlayer;
		}

		if (toSwap != NULL)
			toSwap->SwitchTeam();
	}
}

void CHalfLifeMultiplay::CheckMapConditions(void)
{
	if ((UTIL_FindEntityByClassname(NULL, "func_controlpoint")) != NULL)//ckf
	{
		m_bMapHasControlPoint = true;
		m_iRedLocal = m_iBluLocal = 0;
	}

	if ((UTIL_FindEntityByClassname(NULL, "trigger_resupplyroom")) != NULL)
		m_bMapHasResupplyRoom = true;
	else
		m_bMapHasResupplyRoom = false;
}

void CHalfLifeMultiplay::UpdateTeamScores(void)
{
	MESSAGE_BEGIN(MSG_ALL, gmsgTeamScore);
	WRITE_STRING(GetTeam(TEAM_CT));
	WRITE_SHORT(m_iNumCTWins);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ALL, gmsgTeamScore);
	WRITE_STRING(GetTeam(TEAM_TERRORIST));
	WRITE_SHORT(m_iNumTerroristWins);
	MESSAGE_END();
}

void CHalfLifeMultiplay::SwapAllPlayers(void)
{
	CBaseEntity *pEntity = NULL;

	while ((pEntity = UTIL_FindEntityByClassname(pEntity, "player")) != NULL)
	{
		if (FNullEnt(pEntity->edict()))
			break;

		if (pEntity->pev->flags == FL_DORMANT)
			continue;

		GetClassPtr((CBasePlayer *)pEntity->pev)->SwitchTeam();
	}

	int iTemp = m_iNumTerroristWins;
	m_iNumTerroristWins = m_iNumCTWins;
	m_iNumCTWins = iTemp;
	UpdateTeamScores();
}

extern void ClearBodyQue(void);
extern int CountTeamPlayers(int team);

void CHalfLifeMultiplay::RestartRound(void)
{
	m_iTotalRoundsPlayed++;
	ClearBodyQue();

	CVAR_SET_FLOAT("sv_accelerate", 13);
	CVAR_SET_FLOAT("sv_friction", 4);
	CVAR_SET_FLOAT("sv_stopspeed", 75);

	m_iNumCT = CountTeamPlayers(TEAM_CT);
	m_iNumTerrorist = CountTeamPlayers(TEAM_TERRORIST);

	MESSAGE_BEGIN(MSG_SPEC, gmsgHLTV);
	WRITE_BYTE(0);
	WRITE_BYTE(100 | 128);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_SPEC, gmsgHLTV);
	WRITE_BYTE(0);
	WRITE_BYTE(0);
	MESSAGE_END();

	if (CVAR_GET_FLOAT("mp_autoteambalance") != 0 && m_iUnBalancedRounds >= 1)
		BalanceTeams();

	if (m_iNumCT - m_iNumTerrorist >= 2 || m_iNumTerrorist - m_iNumCT >= 2)
		m_iUnBalancedRounds++;
	else
		m_iUnBalancedRounds = 0;

	if (CVAR_GET_FLOAT("mp_autoteambalance") != 0 && m_iUnBalancedRounds == 1)
		UTIL_ClientPrintAll(HUD_PRINTCENTER, "#Auto_Team_Balance_Next_Round");

	if (m_bCompleteReset)
	{
		if (timelimit.value < 0)
			CVAR_SET_FLOAT("mp_timelimit", 0);

		g_flResetTime = gpGlobals->time;

		if (timelimit.value)
		{			
			g_flTimeLimit = gpGlobals->time + timelimit.value * 60;
			UpdateTimeLimit();
		}

		m_iTotalRoundsPlayed = 0;
		m_iMaxRounds = (int)CVAR_GET_FLOAT("mp_maxrounds");

		if (m_iMaxRounds < 0)
		{
			m_iMaxRounds = 0;
			CVAR_SET_FLOAT("mp_maxrounds", 0);
		}

		m_iWinLimit = (int)CVAR_GET_FLOAT("mp_winlimit");

		if (m_iWinLimit < 0)
		{
			m_iWinLimit = 0;
			CVAR_SET_FLOAT("mp_winlimit", 0);
		}

		m_iNumTerroristWins = 0;
		m_iNumCTWins = 0;
		m_iNumConsecutiveTerroristLoses = 0;
		m_iNumConsecutiveCTLoses = 0;
		UpdateTeamScores();

		for (int i = 1; i <= gpGlobals->maxClients; i++)
		{
			CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex(i);

			if (pPlayer && !FNullEnt(pPlayer->pev))
				pPlayer->Restart();
		}
	}

	m_bRoundTerminating = FALSE;

	ReadMultiplayCvars();

	CheckMapConditions();

	if (m_bCompleteReset)
	{
		m_iNumTerroristWins = 0;
		m_iNumCTWins = 0;
		m_iNumConsecutiveTerroristLoses = 0;
		m_iNumConsecutiveCTLoses = 0;
	}

	CleanUpMap();

	if(m_bMapHasControlPoint)
	{
		CPResetAll();
	}

	//Fire
	FireTargets("game_round_start", g_pWorld, g_pWorld, USE_TOGGLE, 0);

	//Read map params

	CMapInfo *params = (CMapInfo *)UTIL_FindEntityByClassname(NULL, "info_map_settings");

	if (params)
	{
		if(params->m_iSetupCondition >= 0)
			m_iSetupCondition = params->m_iSetupCondition;
		if(params->m_iEndAction >= 0)
			m_iEndAction = params->m_iEndAction;
		if (params->m_iRedDominatedAction >= 0)
			m_iRedDominatedAction = params->m_iRedDominatedAction;
		if (params->m_iBluDominatedAction >= 0)
			m_iBluDominatedAction = params->m_iBluDominatedAction;
		if(params->m_iFreezeTime >= 0)
			m_iFreezeTime = params->m_iFreezeTime;
		if(params->m_iWaitTime >= 0)
			m_iWaitTime = params->m_iWaitTime;
		if(params->m_iSetupTime >= 0)
			m_iSetupTime = params->m_iSetupTime;
		if(params->m_iRoundTime >= 0)
			m_iRoundTime = params->m_iRoundTime;
		if(params->m_iEndTime >= 0)
			m_iEndTime = params->m_iEndTime;
	}


	//Objective Reset
	if(m_bMapHasControlPoint)
	{
		CPSendState();
	}

	m_flIntermissionEndTime = 0;
	m_flIntermissionStartTime = 0;
	m_bLevelInitialized = false;
	m_bCompleteReset = false;
	m_iRoundWinStatus = 0;

	//m_bTimerExpired = false;

	m_fMaxIdlePeriod = 120;//Idle Kick

	if(!m_bFirstConnected && m_iWaitTime > 0)
		SetRoundStatus(ROUND_WAIT);
	else
		SetRoundStatus(m_iSetupCondition);
	SyncRoundTimer();

	if(m_iRoundStatus == ROUND_SETUP && m_iFreezeTime > 0)
	{
		m_bFreezePeriod = true;
		m_flFreezeTimer = gpGlobals->time;
	}

	//Respawn Players

	CBaseEntity *pEntity = NULL;

	while ((pEntity = UTIL_FindEntityByClassname(pEntity, "player")) != NULL)
	{
		if (FNullEnt(pEntity->edict()))
			break;

		if (pEntity->pev->flags == FL_DORMANT)
			continue;

		CBasePlayer *pPlayer = GetClassPtr((CBasePlayer *)pEntity->pev);
		pPlayer->m_iNumSpawns = 0;
		pPlayer->m_bTeamChanged = false;

		if (pPlayer->m_iTeam != TEAM_UNASSIGNED && pPlayer->m_iTeam != TEAM_SPECTATOR)
		{
			//if (pPlayer->m_bIsBombGuy)//Drop Intelligent
				//pPlayer->DropPlayerItem("weapon_c4");
			pPlayer->m_bShouldClearBuild = true;
			pPlayer->RoundRespawn();
			pPlayer->ResetMaxSpeed();
		}
	}
}

void CHalfLifeMultiplay::SetRoundStatus(int iStatus)
{
	switch(iStatus)
	{
	case ROUND_SETUP:
		SetRoundStatus(iStatus, m_iSetupTime);
		break;
	case ROUND_END:
		SetRoundStatus(iStatus, m_iEndTime);
		break;
	case ROUND_WAIT:
		SetRoundStatus(iStatus, m_iWaitTime);
		break;
	default:
		SetRoundStatus(iStatus, m_iRoundTime);
		break;
	}
}

void CHalfLifeMultiplay::SetRoundStatus(int iStatus, float flMaxTime)
{
	m_iRoundStatus = iStatus;

	m_flRoundTimer = gpGlobals->time;

	m_iRoundTimeMax = flMaxTime;
}

BOOL CHalfLifeMultiplay::IsThereABomber(void)
{
	CBasePlayer *pPlayer = NULL;

	while ((pPlayer = (CBasePlayer *)UTIL_FindEntityByClassname(pPlayer, "player")) != NULL)
	{
		if (pPlayer->m_iTeam != TEAM_CT && pPlayer->IsBombGuy())
			return TRUE;
	}

	return FALSE;
}

BOOL CHalfLifeMultiplay::IsThereABomb(void)
{
	BOOL there = FALSE;
	CGrenade *pGrenade = NULL;

	//while ((pGrenade = (CGrenade *)UTIL_FindEntityByClassname(pGrenade, "grenade")) != NULL && !pGrenade->m_bIsC4)
	//{
	//	if (pGrenade->m_bIsC4)
	//	{
	//		there = TRUE;
	//		break;
	//	}
	//}

	if (there || (UTIL_FindEntityByClassname(NULL, "weapon_c4")) != NULL)
		return TRUE;

	return FALSE;
}

BOOL CHalfLifeMultiplay::TeamFull(int team_id)
{
	BOOL result = FALSE;

	switch (team_id)
	{
		case TEAM_TERRORIST:
		{
			if (m_iNumTerrorist >= m_iSpawnPointCount_Terrorist)
				result = TRUE;

			break;
		}

		case TEAM_CT:
		{
			if (m_iNumCT >= m_iSpawnPointCount_CT)
				result = TRUE;

			break;
		}
	}

	return result;
}

BOOL CHalfLifeMultiplay::TeamStacked(int newTeam_id, int curTeam_id)
{
	if (newTeam_id == curTeam_id)
		return FALSE;

	switch (newTeam_id)
	{
		case TEAM_RED:
		{
			if (curTeam_id != TEAM_UNASSIGNED && curTeam_id != TEAM_SPECTATOR)
				return (m_iNumTerrorist + 1) > (m_iNumCT + m_iLimitTeam - 1);

			return (m_iNumTerrorist + 1) > (m_iNumCT + m_iLimitTeam);
		}

		case TEAM_BLUE:
		{
			if (curTeam_id != TEAM_UNASSIGNED && curTeam_id != TEAM_SPECTATOR)
				return (m_iNumCT + 1) > (m_iNumTerrorist + m_iLimitTeam - 1);

			return (m_iNumCT + 1) > (m_iNumTerrorist + m_iLimitTeam);
		}
	}

	return FALSE;
}

#define MAX_INTERMISSION_TIME 120

void CHalfLifeMultiplay::Think(void)
{
	m_VoiceGameMgr.Update(gpGlobals->frametime);

	if ((int)CVAR_GET_FLOAT("sv_clienttrace") != 1)
		CVAR_SET_FLOAT("sv_clienttrace", 1);


	if (CheckGameOver())
		return;

	if (CheckTimeLimit())
		return;

	if (CheckMaxRounds())
		return;

	if (CheckWinLimit())
		return;

	CheckSharedConVars();

	if(m_bFreezePeriod)
	{
		CheckFreezePeriodExpired();
	}

	switch(m_iRoundStatus)
	{
	case ROUND_SETUP:
		CheckSetupPeriodExpired();
		break;
	case ROUND_END:
		CheckEndPeriodExpired();
		break;
	case ROUND_WAIT:
		CheckWaitPeriodExpired();
		break;
	default:
		CheckRoundTimeExpired();
		break;
	}

	CheckLevelInitialized();

	if (gpGlobals->time > m_tmNextPeriodicThink)
	{
		m_tmNextPeriodicThink = gpGlobals->time + 1;
		CheckRestartRound();	
		CheckGameCvar();
	
		if(m_fRedWaveTime)
			m_fRedWaveThink = gpGlobals->time + m_fRedWaveTime;
		else
			m_fRedWaveThink = gpGlobals->time + 10;

		if(m_fBluWaveTime)
			m_fBluWaveThink = gpGlobals->time + m_fBluWaveTime;
		else
			m_fBluWaveThink = gpGlobals->time + 10;
	}

	AnnounceRoundTime();
}

void CHalfLifeMultiplay::CheckWaitPeriodExpired(void)
{
	if (m_iRoundStatus != ROUND_WAIT)
		return;

	if (TimeRemaining() > 0)
		return;

	m_bCompleteReset = TRUE;
	m_bFirstConnected = TRUE;
	RestartRound();
}

void CHalfLifeMultiplay::CheckFreezePeriodExpired(void)
{
	if (m_iFreezeTime - gpGlobals->time + m_flFreezeTimer > 0)
		return;

	FireTargets("game_freeze_expired", g_pWorld, g_pWorld, USE_TOGGLE, 0);

	m_bFreezePeriod = false;

	//SetRoundStatus(ROUND_SETUP);
	//SyncRoundTimer();

	//UTIL_LogPrintf("World triggered \"Round_Start\"\n");

	/*char CT_sentence[40], T_sentence[40];

	switch (RANDOM_LONG(0, 3))
	{
		case 0:
		{
			strncpy(CT_sentence, "%!MRAD_MOVEOUT", sizeof(CT_sentence));
			strncpy(T_sentence, "%!MRAD_MOVEOUT", sizeof(T_sentence));
			break;
		}

		case 1:
		{
			strncpy(CT_sentence, "%!MRAD_LETSGO", sizeof(CT_sentence));
			strncpy(T_sentence, "%!MRAD_LETSGO", sizeof(T_sentence));
			break;
		}

		case 2:
		{
			strncpy(CT_sentence, "%!MRAD_LOCKNLOAD", sizeof(CT_sentence));
			strncpy(T_sentence, "%!MRAD_LOCKNLOAD", sizeof(T_sentence));
			break;
		}

		case 3:
		{
			strncpy(CT_sentence, "%!MRAD_GO", sizeof(CT_sentence));
			strncpy(T_sentence, "%!MRAD_GO", sizeof(T_sentence));
			break;
		}
	}*/

	BOOL bCTPlayed = FALSE, bTPlayed = FALSE;

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex(i);

		if (!pPlayer || pPlayer->pev->flags == FL_DORMANT)
			continue;

		if (pPlayer->m_iJoiningState == JOINED)
		{
			/*if (pPlayer->m_iTeam == TEAM_CT && !bCTPlayed)
			{
				pPlayer->Radio(CT_sentence, NULL);
				bCTPlayed = TRUE;
			}
			else if (pPlayer->m_iTeam == TEAM_TERRORIST && !bCTPlayed)
			{
				pPlayer->Radio(T_sentence, NULL);
				bTPlayed = TRUE;
			}*/

			if (pPlayer->m_iTeam != TEAM_SPECTATOR)
			{
				pPlayer->ResetMaxSpeed();
			}
		}
	}
}

void CHalfLifeMultiplay::CheckSetupPeriodExpired(void)
{
	if (m_iRoundStatus != ROUND_SETUP)
		return;

	if (TimeRemaining() > 0)
		return;

	FireTargets("game_setup_expired", g_pWorld, g_pWorld, USE_TOGGLE, 0);

	SetRoundStatus(ROUND_NORMAL);
	SyncRoundTimer();
}

void CHalfLifeMultiplay::CheckRoundTimeExpired(void)
{
	if (m_iRoundStatus != ROUND_NORMAL && m_iRoundStatus != ROUND_OVERTIME)
		return;

	if (TimeRemaining() > 0)
		return;

	if(m_bMapHasControlPoint)
	{
		BOOL bOverTime = CPCheckOvertime();
		if(bOverTime && m_iRoundStatus == ROUND_NORMAL)
		{
			switch(RANDOM_LONG(0, 3))
			{
			case 0:UTIL_PlayMP3(NULL, "sound/CKF_III/ano/announcer_overtime.mp3");break;
			case 1:UTIL_PlayMP3(NULL, "sound/CKF_III/ano/announcer_overtime2.mp3");break;
			case 2:UTIL_PlayMP3(NULL, "sound/CKF_III/ano/announcer_overtime3.mp3");break;
			case 3:UTIL_PlayMP3(NULL, "sound/CKF_III/ano/announcer_overtime4.mp3");break;
			}
			m_iRoundStatus = ROUND_OVERTIME;
			SyncRoundTimer();
			return;
		}
		else if(bOverTime && m_iRoundStatus == ROUND_OVERTIME)
		{
			return;
		}
		else
		{
			m_iRoundStatus = ROUND_NORMAL;
		}
	}

	FireTargets("game_round_expired", g_pWorld, g_pWorld, USE_TOGGLE, 0);

	//m_bTimerExpired = true;

	switch (m_iEndAction)
	{
	case END_NOTHING:
		break;
	case END_DRAW:
		TerminateRound(m_iEndTime, WINSTATUS_DRAW);
		break;
	case END_RED_WIN:
		TerminateRound(m_iEndTime, WINSTATUS_TERRORIST);
		break;
	case END_BLU_WIN:
		TerminateRound(m_iEndTime, WINSTATUS_CT);
		break;
	case END_SUDDEN_DEATH:
		// TODO NYI
		break;
	}
}

void CHalfLifeMultiplay::CheckEndPeriodExpired(void)
{
	if (m_iRoundStatus != ROUND_END)
		return;

	if (TimeRemaining() > 0)
		return;

	RestartRound();
}

void CHalfLifeMultiplay::CheckLevelInitialized(void)
{
	if (m_bLevelInitialized)
		return;

	m_iSpawnPointCount_CT = 0;
	m_iSpawnPointCount_Terrorist = 0;

	CBaseEntity *pEntity = NULL;

	while ((pEntity = UTIL_FindEntityByClassname(pEntity, "info_player_deathmatch")) != NULL)
		m_iSpawnPointCount_Terrorist++;

	while ((pEntity = UTIL_FindEntityByClassname(pEntity, "info_player_start")) != NULL)
		m_iSpawnPointCount_CT++;

	m_bLevelInitialized = TRUE;
}

void CHalfLifeMultiplay::CheckRestartRound(void)
{
	int iRestartDelay = restartround.value;

	if (!iRestartDelay)
		iRestartDelay = sv_restart.value;

	if (iRestartDelay > 0)
	{
		if (iRestartDelay > 10)
			iRestartDelay = 10;

		UTIL_LogPrintf("World triggered \"Restart_Round_(%i_%s)\"\n", iRestartDelay, iRestartDelay == 1 ? "second" : "seconds");

		if (g_pGameRules)
		{
			UTIL_LogPrintf("Team \"CT\" scored \"%i\" with \"%i\" players\n", g_pGameRules->m_iNumCTWins, g_pGameRules->m_iNumCT);
			UTIL_LogPrintf("Team \"TERRORIST\" scored \"%i\" with \"%i\" players\n", g_pGameRules->m_iNumTerroristWins, g_pGameRules->m_iNumTerrorist);
		}

		UTIL_ClientPrintAll(HUD_PRINTCENTER, "#Game_will_restart_in", UTIL_dtos1(iRestartDelay), iRestartDelay == 1 ? "SECOND" : "SECONDS");
		UTIL_ClientPrintAll(HUD_PRINTCONSOLE, "#Game_will_restart_in", UTIL_dtos1(iRestartDelay), iRestartDelay == 1 ? "SECOND" : "SECONDS");

		m_bCompleteReset = TRUE;
		SetRoundStatus(ROUND_END);
		m_iRoundTimeMax = iRestartDelay;

		CVAR_SET_FLOAT("sv_restartround", 0);
		CVAR_SET_FLOAT("sv_restart", 0);
	}
}

BOOL CHalfLifeMultiplay::CheckGameOver(void)
{
	if (!g_fGameOver)
		return FALSE;

	int time = (int)CVAR_GET_FLOAT("mp_chattime");

	if (time < 1)
		CVAR_SET_STRING("mp_chattime", "1");
	else if (time > MAX_INTERMISSION_TIME)
		CVAR_SET_STRING("mp_chattime", UTIL_dtos1(MAX_INTERMISSION_TIME));

	m_flIntermissionEndTime = m_flIntermissionStartTime + mp_chattime.value;

	if (m_flIntermissionEndTime < gpGlobals->time && (m_iEndIntermissionButtonHit || gpGlobals->time > m_flIntermissionStartTime + MAX_INTERMISSION_TIME))
		ChangeLevel();

	return TRUE;
}

BOOL CHalfLifeMultiplay::CheckTimeLimit(void)
{
	if(m_iRoundStatus != ROUND_END)
		return FALSE;

	if (timelimit.value >= 0)
	{
		if (timelimit.value)
		{
			g_flTimeLimit = g_flResetTime + timelimit.value * 60;

			if (g_flTimeLimit <= gpGlobals->time)
			{
				ALERT(at_console, "Changing maps because time limit has been met\n");
				GoToIntermission();
				return TRUE;
			}
		}
	}
	else
		CVAR_SET_FLOAT("mp_timelimit", 0);

	return FALSE;
}

BOOL CHalfLifeMultiplay::CheckMaxRounds(void)
{
	if (!m_iMaxRounds)
		return FALSE;

	if (m_iTotalRoundsPlayed < m_iMaxRounds)
		return FALSE;

	ALERT(at_console, "Changing maps due to maximum rounds have been met\n");
	GoToIntermission();
	return TRUE;
}

BOOL CHalfLifeMultiplay::CheckWinLimit(void)
{
	if (!m_iWinLimit)
		return FALSE;

	if (m_iNumCTWins < m_iWinLimit && m_iNumTerroristWins < m_iWinLimit)
		return FALSE;

	ALERT(at_console, "Changing maps...one team has won the specified number of rounds\n");
	GoToIntermission();
	return TRUE;
}

extern int gmsgAllowSpec;
extern int gmsgPlayerVars;

void CHalfLifeMultiplay::CheckSharedConVars(void)
{
	if (m_iAllowSpectators != allow_spectators.value)
	{
		m_iAllowSpectators = allow_spectators.value;
		m_iLimitTeam = limitteams.value;

		MESSAGE_BEGIN(MSG_ALL, gmsgAllowSpec);
		WRITE_BYTE(allow_spectators.value);
		MESSAGE_END();
	}

	if (m_iLimitTeam != limitteams.value)
	{
		MESSAGE_BEGIN(MSG_ALL, gmsgPlayerVars);
		WRITE_BYTE(PV_iLimitTeams);
		WRITE_BYTE(limitteams.value);
		MESSAGE_END();
	}	
}

void CHalfLifeMultiplay::CheckGameCvar(void)
{
	if (g_psv_accelerate->value != 13)
		CVAR_SET_FLOAT("sv_accelerate", 13);

	if (g_psv_friction->value != 4)
		CVAR_SET_FLOAT("sv_friction", 4);

	if (g_psv_stopspeed->value != 75)
		CVAR_SET_FLOAT("sv_stopspeed", 75);

	m_iMaxRounds = maxrounds.value;

	if (m_iMaxRounds < 0)
	{
		m_iMaxRounds = 0;
		CVAR_SET_FLOAT("mp_maxrounds", 0);
	}

	m_iWinLimit = winlimit.value;

	if (m_iWinLimit < 0)
	{
		m_iWinLimit = 0;
		CVAR_SET_FLOAT("mp_winlimit", 0);
	}
}

BOOL CHalfLifeMultiplay::IsMultiplayer(void)
{
	return TRUE;
}

BOOL CHalfLifeMultiplay::IsDeathmatch(void)
{
	return TRUE;
}

BOOL CHalfLifeMultiplay::IsCoOp(void)
{
	return (BOOL)gpGlobals->coop;
}

BOOL CHalfLifeMultiplay::FShouldSwitchWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon)
{
	if (!pWeapon->CanDeploy())
		return FALSE;

	if (!pPlayer->m_pActiveItem)
		return TRUE;

	if (!pPlayer->m_iAutoWepSwitch)
		return FALSE;

	if (!pPlayer->m_pActiveItem->CanHolster())
		return FALSE;

	if (pWeapon->iWeight() > pPlayer->m_pActiveItem->iWeight())
		return TRUE;

	return FALSE;
}

BOOL CHalfLifeMultiplay::GetNextBestWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon)
{
	if (!pCurrentWeapon->CanHolster())
		return FALSE;

	int iBestWeight = -1;
	CBasePlayerItem *pBest = NULL;

	for (int i = 0; i < MAX_ITEM_TYPES; i++)
	{
		CBasePlayerItem *pCheck = pPlayer->m_rgpPlayerItems[i];

		while (pCheck)
		{
			if (pCheck->iWeight() > iBestWeight && pCheck != pCurrentWeapon)
			{
				if (pCheck->CanDeploy())
				{
					iBestWeight = pCheck->iWeight();
					pBest = pCheck;
				}
			}

			pCheck = pCheck->m_pNext;
		}
	}

	if (!pBest)
		return FALSE;

	pPlayer->SwitchWeapon(pBest);
	return TRUE;
}

BOOL CHalfLifeMultiplay::ClientCommand(CBasePlayer *pPlayer, const char *pcmd)
{
	return FALSE;
}

BOOL CHalfLifeMultiplay::ClientCommand_DeadOrAlive(CBasePlayer *pPlayer, const char *pcmd)
{
	return m_VoiceGameMgr.ClientCommand(pPlayer, pcmd) != FALSE;
}

BOOL CHalfLifeMultiplay::ClientConnected(edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[128])
{
	m_VoiceGameMgr.ClientConnected(pEntity);

	MESSAGE_BEGIN(MSG_ONE, gmsgTeamScore, NULL, pEntity);
	WRITE_STRING(GetTeam(TEAM_CT));
	WRITE_SHORT(m_iNumCTWins);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ONE, gmsgTeamScore, NULL, pEntity);
	WRITE_STRING(GetTeam(TEAM_TERRORIST));
	WRITE_SHORT(m_iNumTerroristWins);
	MESSAGE_END();
	return TRUE;
}

extern int gmsgSayText;
extern int gmsgGameMode;

void CHalfLifeMultiplay::UpdateGameMode(CBasePlayer *pPlayer)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgGameMode, NULL, pPlayer->edict());
	WRITE_BYTE(0);
	MESSAGE_END();
}

extern float g_flWeaponCheat;
extern int gmsgViewMode;
extern int gmsgCheckModels;
extern int gmsgTeamInfo;
extern int gmsgMetaRender;

void CHalfLifeMultiplay::InitHUD(CBasePlayer *pl)
{
	int i;

	UTIL_LogPrintf("\"%s<%i><%s><>\" entered the game\n", STRING(pl->pev->netname), GETPLAYERUSERID(pl->edict()), GETPLAYERAUTHID(pl->edict()));
	UpdateGameMode(pl);

	if (g_flWeaponCheat == 0)
	{
		//cam_thirdperson = 0 & Cvar_SetValue(cam_command, 0);
		MESSAGE_BEGIN(MSG_ONE, gmsgViewMode, NULL, pl->edict());
		MESSAGE_END();
	}

	//this is unused in CS client
	//MESSAGE_BEGIN(MSG_ONE, gmsgCheckModels, NULL, pl->edict());
	//MESSAGE_END();

	pl->pev->frags = 0;
	pl->m_iDominates = 0;
	pl->SendScoreInfo();

	cvar_t *cvar_hostname = CVAR_GET_POINTER("hostname");
	if(cvar_hostname)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgServerName, NULL, pl->edict());
		WRITE_STRING(cvar_hostname->string);
		MESSAGE_END();
	}

	SendMOTDToClient(pl);

	for (i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer *plr = (CBasePlayer *)UTIL_PlayerByIndex(i);

		if (plr)
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgScoreInfo, NULL, pl->edict());
			WRITE_BYTE(i);
			WRITE_SHORT(plr->pev->frags);
			WRITE_SHORT(plr->m_Stats.iDeath);
			WRITE_BYTE(plr->m_iDominates);
			MESSAGE_END();
		}
	}

	MESSAGE_BEGIN(MSG_ONE, gmsgTeamScore, NULL, pl->edict());
	WRITE_STRING(GetTeam(TEAM_TERRORIST));
	WRITE_SHORT(m_iNumTerroristWins);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ONE, gmsgTeamScore, NULL, pl->edict());
	WRITE_STRING(GetTeam(TEAM_CT));
	WRITE_SHORT(m_iNumCTWins);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ONE, gmsgAllowSpec, NULL, pl->edict());
	WRITE_BYTE(allow_spectators.value);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ONE, gmsgPlayerVars, NULL, pl->edict());
	WRITE_BYTE(PV_iLimitTeams);
	WRITE_BYTE(limitteams.value);
	MESSAGE_END();

	if (g_fGameOver)
	{
		MESSAGE_BEGIN(MSG_ONE, SVC_INTERMISSION, NULL, pl->edict());
		MESSAGE_END();
	}

	for (i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer *plr = (CBasePlayer *)UTIL_PlayerByIndex(i);

		if (plr)
		{
			MESSAGE_BEGIN(MSG_ONE, gmsgTeamInfo, NULL, pl->edict());
			WRITE_BYTE(ENTINDEX(plr->edict()));
			WRITE_STRING(GetTeam(plr->m_iTeam));
			MESSAGE_END();

			plr->SetScoreboardAttributes(pl);
		}
	}

	for(i = 0; i < m_NoBuildZone.Count(); ++i)
	{
		edict_t *ed = m_NoBuildZone[i];
		MESSAGE_BEGIN(MSG_ONE, gmsgMapObject, NULL, pl->edict());
		WRITE_BYTE(MAP_NOBUILDABLE);
		WRITE_BYTE(ed->v.team);
		WRITE_COORD(ed->v.origin.x + ed->v.mins.x);
		WRITE_COORD(ed->v.origin.y + ed->v.mins.y);
		WRITE_COORD(ed->v.origin.z + ed->v.mins.z);
		WRITE_COORD(ed->v.origin.x + ed->v.maxs.x);
		WRITE_COORD(ed->v.origin.y + ed->v.maxs.y);
		WRITE_COORD(ed->v.origin.z + ed->v.maxs.z);
		WRITE_STRING(STRING(ed->v.model));
		MESSAGE_END();
	}

	for(i = 0; i < m_ShadowManager.Count(); ++i)
	{
		shadow_manager_t *mgr = &m_ShadowManager[i];
		MESSAGE_BEGIN(MSG_ONE, gmsgMetaRender, NULL, pl->edict());
		WRITE_BYTE(METARENDER_SHADOWMGR);
		WRITE_COORD(mgr->angles.x);
		WRITE_COORD(mgr->angles.y);
		WRITE_COORD(mgr->angles.z);
		WRITE_COORD(mgr->radius);
		WRITE_COORD(mgr->fard);
		WRITE_COORD(mgr->scale);
		WRITE_SHORT(mgr->texsize);
		WRITE_STRING(mgr->affectmodel);
		MESSAGE_END();
	}

	CClientFog *pFog = (CClientFog *)UTIL_FindEntityByClassname(NULL, "env_fog");

	if (pFog)
	{
		int r = pFog->pev->rendercolor[0];
		int g = pFog->pev->rendercolor[1];
		int b = pFog->pev->rendercolor[2];
		int density_y = pFog->pev->rendercolor[3];

		if (r > 255)
			r = 255;
		else if (r < 0)
			r = 0;

		if (g > 255)
			g = 255;
		else if (g < 0)
			g = 0;

		if (b > 255)
			b = 255;
		else if (b < 0)
			b = 0;

		MESSAGE_BEGIN(MSG_ONE, gmsgFog, NULL, pl->edict());
		WRITE_BYTE(r);
		WRITE_BYTE(g);
		WRITE_BYTE(b);
		WRITE_BYTE((int)pFog->m_fDensity << 24);
		WRITE_BYTE((int)pFog->m_fDensity << 16);
		WRITE_BYTE((int)pFog->m_fDensity << 8);
		WRITE_BYTE((int)pFog->m_fDensity);
		MESSAGE_END();
	}
}

void CHalfLifeMultiplay::ClientDisconnected(edict_t *pClient)
{
	if (pClient)
	{
		CBasePlayer *pPlayer = (CBasePlayer *)CBaseEntity::Instance(pClient);

		if (pPlayer)
		{
			pPlayer->m_bDisconnect = true;
			pPlayer->pev->deadflag = DEAD_DEAD;
			pPlayer->pev->frags = 0;
			pPlayer->m_iClass = 0;
			pPlayer->m_iTeam = 0;

			MESSAGE_BEGIN(MSG_ALL, gmsgScoreInfo);
			WRITE_BYTE(ENTINDEX(pClient));
			WRITE_SHORT(0);
			WRITE_SHORT(0);
			WRITE_BYTE(0);
			MESSAGE_END();

			MESSAGE_BEGIN(MSG_ALL, gmsgTeamInfo);
			WRITE_BYTE(ENTINDEX(pClient));
			WRITE_STRING(GetTeam(TEAM_UNASSIGNED));
			MESSAGE_END();

			pPlayer->SetScoreboardAttributes();

			//if (pPlayer->m_bIsBombGuy == TRUE)
			//	pPlayer->DropPlayerItem("weapon_c4");

			//if (pPlayer->m_bHasDefuseKit == TRUE)
			//	pPlayer->DropPlayerItem("item_thighpack");

			pPlayer->m_iVotingKikedPlayerId = 0;

			if (pPlayer->m_iVotingMapId)
			{
				m_rgiVotingMapCount[pPlayer->m_iVotingMapId]--;

				if (m_rgiVotingMapCount[pPlayer->m_iVotingMapId] < 0)
					m_rgiVotingMapCount[pPlayer->m_iVotingMapId] = 0;
			}

			FireTargets("game_playerleave", pPlayer, pPlayer, USE_TOGGLE, 0);
			UTIL_LogPrintf("\"%s<%i><%s><%s>\" disconnected\n", STRING(pPlayer->pev->netname), GETPLAYERUSERID(pPlayer->edict()), GETPLAYERAUTHID(pPlayer->edict()), GetTeam(pPlayer->m_iTeam));
			pPlayer->RemoveAllItems(TRUE);

			if (pPlayer->m_pLinkedEntity)
				pPlayer->m_pLinkedEntity->SUB_Remove();

			CBasePlayer *client = NULL;

			while ((client = (CBasePlayer *)UTIL_FindEntityByClassname(client, "player")) != NULL)
			{
				if (FNullEnt(client->edict()))
					break;

				if (!client->pev || client == pPlayer || client->m_hObserverTarget == pPlayer)
					continue;

				int iMode = client->pev->iuser1;
				client->pev->iuser1 = 0;
				client->Observer_SetMode(iMode);
			}
		}
	}

	CheckWinConditions();
}

float CHalfLifeMultiplay::FlPlayerFallDamage(CBasePlayer *pPlayer)
{
	pPlayer->m_flFallVelocity -= PLAYER_MAX_SAFE_FALL_SPEED;
	return pPlayer->m_flFallVelocity * DAMAGE_FOR_FALL_SPEED * 1.25;
}

BOOL CHalfLifeMultiplay::FPlayerCanTakeDamage(CBasePlayer *pPlayer, CBaseEntity *pAttacker)
{
	if (!pAttacker)
		return TRUE;

	if (PlayerRelationship(pPlayer, pAttacker) != GR_TEAMMATE)
		return TRUE;

	if (CVAR_GET_FLOAT("mp_friendlyfire") != 0)
		return TRUE;

	if (pAttacker == pPlayer)
		return TRUE;

	return FALSE;
}

extern void ShowVGUIMenu(CBasePlayer *pPlayer, int type, int bitsKey, char *string);

void CHalfLifeMultiplay::PlayerThink(CBasePlayer *pPlayer)
{
	if (g_fGameOver)
	{
		if (pPlayer->m_afButtonPressed & (IN_DUCK | IN_ATTACK | IN_ATTACK2 | IN_USE | IN_JUMP))
			m_iEndIntermissionButtonHit = TRUE;

		pPlayer->m_afButtonPressed = 0;
		pPlayer->pev->button = 0;
		pPlayer->m_afButtonReleased = 0;
	}

	if (pPlayer->m_iMenu != MENU_TEAM && pPlayer->m_iJoiningState == SHOWTEAMSELECT)//hz
	{
		//if (!allow_spectators.value)
		//	ShowVGUIMenu(pPlayer, MENU_TEAM, KEY_1 | KEY_3 | KEY_4, "");
		//else
		//	ShowVGUIMenu(pPlayer, MENU_TEAM, KEY_1 | KEY_2 | KEY_3 | KEY_4, "");
		pPlayer->m_iJoiningState = PICKINGTEAM;
	}
}

void CHalfLifeMultiplay::PlayerSpawn(CBasePlayer *pPlayer)
{
	if (pPlayer->m_bJustConnected)
		return;

	pPlayer->pev->weapons |= (1 << WEAPON_SUIT);

	BOOL addDefault = TRUE;
	CBaseEntity *pWeaponEntity = NULL;

	while ((pWeaponEntity = UTIL_FindEntityByClassname(pWeaponEntity, "game_player_equip")) != NULL)
	{
		pWeaponEntity->Touch(pPlayer);
		addDefault = FALSE;
	}

	pPlayer->GiveDefaultItems();
}

BOOL CHalfLifeMultiplay::FPlayerCanRespawn(CBasePlayer *pPlayer)
{
	if (pPlayer->m_iNumSpawns > 0)
		return FALSE;

	m_iNumCT = CountTeamPlayers(TEAM_CT);
	m_iNumTerrorist = CountTeamPlayers(TEAM_TERRORIST);

	if (m_iNumTerrorist > 0 && m_iNumCT > 0 && !(m_iRoundStatus >= ROUND_WAIT && m_iRoundStatus <= ROUND_SETUP))
	{
		if (fadetoblack.value)
			UTIL_ScreenFade(pPlayer, Vector(0, 0, 0), 3, 3, 255, FFADE_OUT | FFADE_STAYOUT);

		return FALSE;
	}

	//if (pPlayer->m_iJoiningState != JOINED)
	//	return FALSE;

	if (pPlayer->m_iMenu == MENU_CLASS_RED || pPlayer->m_iMenu == MENU_CLASS_BLU || pPlayer->m_iMenu == MENU_TEAM)
		return FALSE;

	return TRUE;
}

float CHalfLifeMultiplay::FlPlayerSpawnTime(CBasePlayer *pPlayer)
{
	return gpGlobals->time;
}

BOOL CHalfLifeMultiplay::AllowAutoTargetCrosshair(void)
{
	static cvar_t *mp_autocrosshair = NULL;

	if (!mp_autocrosshair)
		mp_autocrosshair = CVAR_GET_POINTER("mp_autocrosshair");

	if (mp_autocrosshair)
		return mp_autocrosshair->value != 0;

	return FALSE;
}

void CHalfLifeMultiplay::BuildKilled(CBaseBuildable *pVictim, entvars_t *pevKiller, entvars_t *pevInflictor)
{
	CBasePlayer *plKiller = NULL;
	CBaseEntity *pKiller = CBaseEntity::Instance(pevKiller);

	if (pKiller && pKiller->Classify() == CLASS_PLAYER)
	{
		plKiller = (CBasePlayer *)pKiller;
	}

	FireTargets("game_builddie", pVictim, pVictim, USE_TOGGLE, 0);

	if (plKiller && pevKiller != pVictim->pev)
	{
		BuildDeathNotice(pVictim, pevKiller, pevInflictor);

		plKiller->m_Stats.iDemolish ++;
		plKiller->SendStatsInfo(STATS_DEMOLISH);
		pevKiller->frags += IPointsForKill(plKiller, pVictim);

		CBaseEntity *pInflictor = CBaseEntity::Instance(pevInflictor);

		if(pInflictor)
		{
			if(pInflictor->Classify() == CLASS_BUILDABLE)
			{
				pInflictor->pev->frags ++;
			}
		}

		FireTargets("game_buildkill", pKiller, pKiller, USE_TOGGLE, 0);

		//Update Killer
		plKiller->SendScoreInfo();
	}
}

void CHalfLifeMultiplay::SapperKilled(CBuildSapper *pVictim, entvars_t *pevKiller, entvars_t *pevInflictor)
{
	CBasePlayer *plKiller = NULL;
	CBaseEntity *pKiller = CBaseEntity::Instance(pevKiller);

	if (pKiller && pKiller->Classify() == CLASS_PLAYER)
	{
		plKiller = (CBasePlayer *)pKiller;
	}

	FireTargets("game_sapperdie", pVictim, pVictim, USE_TOGGLE, 0);

	if (plKiller && pevKiller != pVictim->pev)
	{
		BuildDeathNotice(pVictim, pevKiller, pevInflictor);

		plKiller->m_Stats.iBonus ++;
		plKiller->SendStatsInfo(STATS_BONUS);
		pevKiller->frags += IPointsForKill(plKiller, pVictim);

		CBaseEntity *pInflictor = CBaseEntity::Instance(pevInflictor);

		FireTargets("game_sapperkill", pKiller, pKiller, USE_TOGGLE, 0);

		//Update Killer
		plKiller->SendScoreInfo();
	}
}

void CHalfLifeMultiplay::BuildDeathNotice(CBaseEntity *pVictim, entvars_t *pevKiller, entvars_t *pevInflictor)
{
	const char *killer_weapon_name = "world";
	int killer_index = 0;

	if (pevKiller->flags & FL_CLIENT)
	{
		killer_index = ENTINDEX(ENT(pevKiller));

		if (pevInflictor)
		{
			if (pevInflictor == pevKiller)
			{
				CBasePlayer *pPlayer = (CBasePlayer *)CBaseEntity::Instance(pevKiller);

				if (pPlayer->m_pActiveItem)
					killer_weapon_name = pPlayer->m_pActiveItem->pszName();
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
	else if (!strncmp(killer_weapon_name, "func_", 5))
		killer_weapon_name += 5;
	else if (!strcmp(killer_weapon_name, "pj_senrocket"))
		killer_weapon_name = "sentry3";
	else if (!strncmp(killer_weapon_name, "pj_", 3))
	{
		killer_weapon_name += 3;
		CBaseEntity *pInflictor = CBaseEntity::Instance(pevInflictor);
		if(pInflictor && pInflictor->Classify() == CLASS_PROJECTILE)
		{
			CGrenade *pGrenade = (CGrenade *)pInflictor;
			if(pGrenade->IsDeflected())
			{
				if(!strncmp(killer_weapon_name, "rocket", 6) || !strncmp(killer_weapon_name, "senrocket", 9))
				{
					killer_weapon_name = "defrocket";
				}
				else if(!strncmp(killer_weapon_name, "grenade", 7))
				{
					killer_weapon_name = "defgrenade";
				}
				else if(!strncmp(killer_weapon_name, "sticky", 6))
				{
					killer_weapon_name = "defsticky";
				}
			}
		}
	}
	else if (!strncmp(killer_weapon_name, "buildable_", 10))
	{
		if(!strcmp(killer_weapon_name, "buildable_sentry"))
		{
			CBaseBuildable *pBuild = (CBaseBuildable *)CBaseEntity::Instance(pevInflictor);
			if(pBuild)
			{
				if(pBuild->m_iLevel == 1)
					killer_weapon_name = "sentry1";
				else if(pBuild->m_iLevel == 2)
					killer_weapon_name = "sentry2";
				else
					killer_weapon_name = "sentry3";
			}
		}
		else
			killer_weapon_name += 10;
	}

	int victim_index = 0;

	CBaseBuildable *pBuild = dynamic_cast<CBaseBuildable *>(pVictim);
	CBuildSapper *pSapper = dynamic_cast<CBuildSapper *>(pVictim);
	if(pBuild)
	{
		if(!pBuild->m_pPlayer)
			return;

		victim_index = pBuild->m_pPlayer->entindex();

		if(!victim_index || victim_index > gpGlobals->maxClients)
			return;

		MESSAGE_BEGIN(MSG_ALL, gmsgBuildDeath);
		WRITE_BYTE(killer_index);
		WRITE_BYTE(0);//assister = 0
		WRITE_BYTE(victim_index);
		WRITE_BYTE(pBuild->GetBuildClass());
		WRITE_STRING(killer_weapon_name);
		MESSAGE_END();
	}
	else if(pSapper)
	{
		if(!pSapper->pev->owner)
			return;

		victim_index = ENTINDEX(pSapper->pev->owner);

		if(!victim_index || victim_index > gpGlobals->maxClients)
			return;

		MESSAGE_BEGIN(MSG_ALL, gmsgBuildDeath);
		WRITE_BYTE(killer_index);
		WRITE_BYTE(0);//assister = 0
		WRITE_BYTE(victim_index);
		WRITE_BYTE(5);
		WRITE_STRING(killer_weapon_name);
		MESSAGE_END();
	}
}

float CHalfLifeMultiplay::IPointsForKill(CBasePlayer *pAttacker, CBaseEntity *pKilled)
{
	if(pKilled->Classify() == CLASS_BUILDABLE)
		return 0.5;
	if(pKilled->Classify() == CLASS_PROJECTILE)
		return 0.25;
	return 1;
}

void CHalfLifeMultiplay::PlayerKilled(CBasePlayer *pVictim, entvars_t *pevKiller, entvars_t *pevInflictor)
{
	//Set Victim
	pVictim->m_afPhysicsFlags &= ~PFLAG_ONTRAIN;
	pVictim->m_Stats.iDeath ++;
	pVictim->SendStatsInfo(STATS_DEATH);
	pVictim->m_bNotKilled = FALSE;
	pVictim->m_iTrain = TRAIN_NEW | TRAIN_OFF;
	SET_VIEW(ENT(pVictim->pev), ENT(pVictim->pev));

	CBasePlayer *plKiller = NULL;
	CBasePlayer *plAssister = NULL;
	CBaseEntity *pKiller = CBaseEntity::Instance(pevKiller);

	if (pKiller && pKiller->Classify() == CLASS_PLAYER)
	{
		plKiller = (CBasePlayer *)pKiller;
	}
	else if (pKiller && pKiller->Classify() == CLASS_VEHICLE)
	{
		pKiller = (CBasePlayer *)((CFuncVehicle *)pKiller)->m_pDriver;

		if (pKiller)
		{
			pevKiller = pKiller->pev;
			plKiller = (CBasePlayer *)pKiller;
		}
	}

	FireTargets("game_playerdie", pVictim, pVictim, USE_TOGGLE, 0);

	if(plKiller)
	{
		plAssister = GetAssister(plKiller, pVictim);
	}

	if(plAssister)
	{
		//Add frags
		plAssister->AddPoints(0.5, TRUE);
		plAssister->m_Stats.iAssist ++;
		plAssister->SendStatsInfo(STATS_ASSIST);

		//Dominate&Nemesis
		DominateNemesis(plAssister, pVictim);

		DeathNotice(pVictim, pevKiller, pevInflictor, plAssister->pev);
	}
	else
	{
		DeathNotice(pVictim, pevKiller, pevInflictor, NULL);
	}

	if(plKiller && pVictim->pev != pevKiller)
	{
		//Dominate&Nemesis
		DominateNemesis(plKiller, pVictim);

		CalcPoints(plKiller, pevInflictor, pVictim);
		FireTargets("game_playerkill", pKiller, pKiller, USE_TOGGLE, 0);
	}

	pVictim->SendScoreInfo();


	if (plKiller && plKiller != pVictim)
	{
		plKiller->SendScoreInfo();

		plKiller->m_flNextDecalTime = gpGlobals->time;
	}
}

CBasePlayer *CHalfLifeMultiplay::GetAssister(CBasePlayer *pKiller, CBasePlayer *pVictim)
{
	CBasePlayer *pAssister;
	CBasePlayer *pPlayer;

	int iAmount;
	int iHighest;
	
	pAssister = pKiller->m_pHealer;

	iAmount = 0;
	iHighest = -1;
	if(!pAssister)
	{
		iAmount = 0;
		iHighest = -1;
		for(int i = 1; i <= gpGlobals->maxClients; i++)
		{
			pPlayer = (CBasePlayer *)UTIL_PlayerByIndex(i);
			if (!pPlayer || !FNullEnt(pPlayer->pev)) continue;
			if(pPlayer->m_iTeam != pKiller->m_iTeam) continue;
			if(pPlayer == pKiller) continue;
			if(pPlayer == pVictim) continue;

			//find the teammate the highest damage done by which
			iAmount = pVictim->DmgRecord_Get(pPlayer, 30.0f);
			if(iHighest < 0 || iHighest < iAmount)
			{
				iHighest = iAmount;
				pAssister = pPlayer;
			}
		}
	}

	return pAssister;
}

void CHalfLifeMultiplay::DominateNemesis(CBasePlayer *pKiller, CBasePlayer *pVictim)
{
	if(pVictim->m_Dominate[pKiller->entindex()] >= 4)//Revenge
	{
		pVictim->m_Dominate[pKiller->entindex()] = 0;
		//UTIL_PlayWAV(pKiller, "CKF_III/tf_revenge.wav");
		pKiller->m_Stats.iRevenge ++;
		pKiller->SendStatsInfo(STATS_REVENGE);

		pVictim->UpdateDominate();
		pVictim->SendScoreInfo();

		//UTIL_PlayWAV(pVictim, "CKF_III/tf_revenge.wav");
		MESSAGE_BEGIN(MSG_ALL, gmsgRevenge);
		WRITE_BYTE(pKiller->entindex());
		WRITE_BYTE(pVictim->entindex());
		MESSAGE_END();
	}
	else if(pVictim->m_Dominate[pKiller->entindex()] > 0)//no continous kill
	{
		pVictim->m_Dominate[pKiller->entindex()] = 0;
	}
	pKiller->m_Dominate[pVictim->entindex()] ++;

	if(pKiller->m_Dominate[pVictim->entindex()] >= 4)
	{
		if(pKiller->m_Dominate[pVictim->entindex()] == 4)
		{
			//UTIL_PlayWAV(pKiller, "CKF_III/tf_domination.wav");
			pKiller->UpdateDominate();
			pKiller->SendScoreInfo();

			//UTIL_PlayWAV(pVictim, "CKF_III/tf_nemesis.wav");
			MESSAGE_BEGIN(MSG_ALL, gmsgDominate);
			WRITE_BYTE(pKiller->entindex());
			WRITE_BYTE(pVictim->entindex());
			MESSAGE_END();
		}

		pKiller->m_Stats.iDominate ++;
		pKiller->SendStatsInfo(STATS_DOMINATE);		
	}
}

void CHalfLifeMultiplay::CalcPoints(CBasePlayer *pAttacker, entvars_t *pevInflictor, CBasePlayer *pKilled)
{
	pAttacker->pev->frags += IPointsForKill(pAttacker, pKilled);
	pAttacker->m_Stats.iKill ++;
	pAttacker->SendStatsInfo(STATS_KILL);

	if(pKilled->m_bHeadShotKilled)
	{
		if(!strcmp(STRING(pevInflictor->classname), "weapon_sniperifle"))
		{
			pAttacker->AddPoints(0.5, TRUE);
			pAttacker->m_Stats.iHeadshot ++;
			pAttacker->SendStatsInfo(STATS_HEADSHOT);
		}
	}

	if(pKilled->m_bBackStabKilled)
	{
		pAttacker->AddPoints(1, TRUE);
		pAttacker->m_Stats.iBackstab ++;
		pAttacker->SendStatsInfo(STATS_BACKSTAB);
	}

	if(pAttacker->m_iClass == CLASS_SCOUT && pKilled->m_iClass == CLASS_MEDIC && pKilled->m_pActiveItem)
	{
		if(pKilled->m_pActiveItem->iItemSlot() == WEAPON_SLOT_SECONDARY)
		{
			CMedigun *pMedigun = (CMedigun *)pKilled->m_pActiveItem;
			if(pMedigun->m_bHealing)
			{	
				pAttacker->AddPoints(1, TRUE);
				pAttacker->m_Stats.iBonus ++;
				pAttacker->SendStatsInfo(STATS_BONUS);
			}
		}
	}

	CBaseEntity *pInflictor = CBaseEntity::Instance(pevInflictor);
	if(pInflictor)
	{
		if(pInflictor->Classify() == CLASS_BUILDABLE)
		{
			pInflictor->pev->frags ++;
		}
	}
}

void CHalfLifeMultiplay::DeathNotice(CBasePlayer *pVictim, entvars_t *pevKiller, entvars_t *pevInflictor, entvars_t *pevAssister)
{
	CBaseEntity *pKiller = CBaseEntity::Instance(pevKiller);

	const char *killer_weapon_name = "world";
	int killer_index = 0;

	if (pevKiller->flags & FL_CLIENT)
	{
		killer_index = ENTINDEX(ENT(pevKiller));

		if (pevInflictor)
		{
			if (pevInflictor == pevKiller)
			{
				CBasePlayer *plKiller = (CBasePlayer *)pKiller;
				if (plKiller->m_pActiveItem)
					killer_weapon_name = plKiller->m_pActiveItem->pszName();
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
	else if (!strncmp(killer_weapon_name, "func_", 5))
		killer_weapon_name += 5;
	else if (!strncmp(killer_weapon_name, "pj_", 3))
	{
		killer_weapon_name += 3;
		CBaseEntity *pInflictor = CBaseEntity::Instance(pevInflictor);
		if(pInflictor && pInflictor->Classify() == CLASS_PROJECTILE)
		{
			CGrenade *pGrenade = (CGrenade *)pInflictor;
			if(pGrenade->IsDeflected())
			{
				if(!strncmp(killer_weapon_name, "rocket", 6) || !strncmp(killer_weapon_name, "senrocket", 9))
				{
					killer_weapon_name = "defrocket";
				}
				else if(!strncmp(killer_weapon_name, "grenade", 7))
				{
					killer_weapon_name = "defgrenade";
				}
				else if(!strncmp(killer_weapon_name, "sticky", 6))
				{
					killer_weapon_name = "defsticky";
				}
			}
		}
	}
	else if (!strncmp(killer_weapon_name, "buildable_", 10))
	{
		if(!strcmp(killer_weapon_name, "buildable_sentry"))
		{
			CBaseBuildable *pBuild = (CBaseBuildable *)CBaseEntity::Instance(pevInflictor);
			if(pBuild)
			{
				if(pBuild->m_iLevel == 1)
					killer_weapon_name = "sentry1";
				else if(pBuild->m_iLevel == 2)
					killer_weapon_name = "sentry2";
				else
					killer_weapon_name = "sentry3";
			}
		}
		else
			killer_weapon_name += 10;
	}

	if(pVictim->m_bBackStabKilled && !strcmp(killer_weapon_name, "butterfly"))
		killer_weapon_name = "backstab";
	else if(pVictim->m_bHeadShotKilled && pVictim->m_bCritKilled && !strcmp(killer_weapon_name, "sniperifle"))
		killer_weapon_name = "headshot";

	BOOL bCritKill = FALSE;

	if (pVictim->m_bCritKilled)
		bCritKill = TRUE;

	MESSAGE_BEGIN(MSG_ALL, gmsgDeathMsg);
	WRITE_BYTE(killer_index);
	if(pevAssister)
		WRITE_BYTE(ENTINDEX(ENT(pevAssister)));
	else
		WRITE_BYTE(0);
	WRITE_BYTE(ENTINDEX(pVictim->edict()));
	WRITE_BYTE(bCritKill);
	WRITE_STRING(killer_weapon_name);
	MESSAGE_END();

	if (pVictim->pev == pevKiller)
	{
		UTIL_LogPrintf("\"%s<%i><%s><%s>\" committed suicide with \"%s\"\n", STRING(pVictim->pev->netname), GETPLAYERUSERID(pVictim->edict()), GETPLAYERAUTHID(pVictim->edict()), GetTeam(pVictim->m_iTeam), killer_weapon_name);
	}
	else if (pevKiller->flags & FL_CLIENT)
	{
		CBasePlayer *plKiller = (CBasePlayer *)pKiller;
		UTIL_LogPrintf("\"%s<%i><%s><%s>\" killed \"%s<%i><%s><%s>\" with \"%s\"\n", STRING(pevKiller->netname), GETPLAYERUSERID(ENT(pevKiller)), GETPLAYERAUTHID(ENT(pevKiller)), GetTeam(plKiller->m_iTeam), STRING(pVictim->pev->netname), GETPLAYERUSERID(pVictim->edict()), GETPLAYERAUTHID(pVictim->edict()), GetTeam(pVictim->m_iTeam), killer_weapon_name);
	}
	else
		UTIL_LogPrintf("\"%s<%i><%s><%s>\" committed suicide with \"%s\" (world)\n", STRING(pVictim->pev->netname), GETPLAYERUSERID(pVictim->edict()), GETPLAYERAUTHID(pVictim->edict()), GetTeam(pVictim->m_iTeam), killer_weapon_name);

	MESSAGE_BEGIN(MSG_SPEC, SVC_DIRECTOR);
	WRITE_BYTE(9);
	WRITE_BYTE(DRC_CMD_EVENT);
	WRITE_SHORT(ENTINDEX(pVictim->edict()));

	if (pevInflictor)
		WRITE_SHORT(ENTINDEX(ENT(pevInflictor)));
	else
		WRITE_SHORT(ENTINDEX(ENT(pevKiller)));

	if (bCritKill)
		WRITE_LONG(15 | DRC_FLAG_DRAMATIC | DRC_FLAG_SLOWMOTION);
	else
		WRITE_LONG(7 | DRC_FLAG_DRAMATIC);

	MESSAGE_END();
}

void CHalfLifeMultiplay::PlayerGotWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon)
{
}

float CHalfLifeMultiplay::FlWeaponRespawnTime(CBasePlayerItem *pWeapon)
{
	return gpGlobals->time + WEAPON_RESPAWN_TIME;
}

#define ENTITY_INTOLERANCE 100

float CHalfLifeMultiplay::FlWeaponTryRespawn(CBasePlayerItem *pWeapon)
{
	if (pWeapon && pWeapon->m_iId && (pWeapon->iFlags() & ITEM_FLAG_LIMITINWORLD))
	{
		if (NUMBER_OF_ENTITIES() < (gpGlobals->maxEntities - ENTITY_INTOLERANCE))
			return 0;

		return FlWeaponRespawnTime(pWeapon);
	}

	return 0;
}

Vector CHalfLifeMultiplay::VecWeaponRespawnSpot(CBasePlayerItem *pWeapon)
{
	return pWeapon->pev->origin;
}

int CHalfLifeMultiplay::WeaponShouldRespawn(CBasePlayerItem *pWeapon)
{
	if (pWeapon->pev->spawnflags & SF_NORESPAWN)
		return GR_WEAPON_RESPAWN_NO;

	return GR_WEAPON_RESPAWN_YES;
}

BOOL CHalfLifeMultiplay::CanHavePlayerItem(CBasePlayer *pPlayer, CBasePlayerItem *pItem)
{
	return CGameRules::CanHavePlayerItem(pPlayer, pItem);
}

BOOL CHalfLifeMultiplay::CanHaveItem(CBasePlayer *pPlayer, CItem *pItem)
{
	return TRUE;
}

void CHalfLifeMultiplay::PlayerGotItem(CBasePlayer *pPlayer, CItem *pItem)
{
}

int CHalfLifeMultiplay::ItemShouldRespawn(CItem *pItem)
{
	if (pItem->pev->spawnflags & SF_NORESPAWN)
		return GR_ITEM_RESPAWN_NO;

	return GR_ITEM_RESPAWN_YES;
}

float CHalfLifeMultiplay::FlItemRespawnTime(CItem *pItem)
{
	return gpGlobals->time + ITEM_RESPAWN_TIME;
}

Vector CHalfLifeMultiplay::VecItemRespawnSpot(CItem *pItem)
{
	return pItem->pev->origin;
}

void CHalfLifeMultiplay::PlayerGotAmmo(CBasePlayer *pPlayer, char *szName, int iCount)
{
}

BOOL CHalfLifeMultiplay::IsAllowedToSpawn(CBaseEntity *pEntity)
{
	return TRUE;
}

int CHalfLifeMultiplay::AmmoShouldRespawn(CBasePlayerAmmo *pAmmo)
{
	if (pAmmo->pev->spawnflags & SF_NORESPAWN)
		return GR_AMMO_RESPAWN_NO;

	return GR_AMMO_RESPAWN_YES;
}

float CHalfLifeMultiplay::FlAmmoRespawnTime(CBasePlayerAmmo *pAmmo)
{
	return gpGlobals->time + AMMO_RESPAWN_TIME;
}

Vector CHalfLifeMultiplay::VecAmmoRespawnSpot(CBasePlayerAmmo *pAmmo)
{
	return pAmmo->pev->origin;
}

float CHalfLifeMultiplay::FlHealthChargerRechargeTime(void)
{
	return 60;
}

float CHalfLifeMultiplay::FlHEVChargerRechargeTime(void)
{
	return 30;
}

int CHalfLifeMultiplay::DeadPlayerWeapons(CBasePlayer *pPlayer)
{
	return GR_PLR_DROP_GUN_ACTIVE;
}

int CHalfLifeMultiplay::DeadPlayerAmmo(CBasePlayer *pPlayer)
{
	return GR_PLR_DROP_AMMO_ACTIVE;
}

edict_t *CHalfLifeMultiplay::GetPlayerSpawnSpot(CBasePlayer *pPlayer)
{
	edict_t *pentSpawnSpot = CGameRules::GetPlayerSpawnSpot(pPlayer);

	if (IsMultiplayer() && pentSpawnSpot->v.target)
		FireTargets(STRING(pentSpawnSpot->v.target), pPlayer, pPlayer, USE_TOGGLE, 0);

	return pentSpawnSpot;
}

int CHalfLifeMultiplay::PlayerRelationship(CBaseEntity *pPlayer, CBaseEntity *pTarget)
{
	if (!pPlayer || !pTarget)
		return GR_NOTTEAMMATE;

	if (!pTarget->IsPlayer())
		return GR_NOTTEAMMATE;

	if (GetClassPtr((CBasePlayer *)pPlayer->pev)->m_iTeam != GetClassPtr((CBasePlayer *)pTarget->pev)->m_iTeam)
		return GR_NOTTEAMMATE;

	return GR_TEAMMATE;
}

BOOL CHalfLifeMultiplay::FAllowMonsters(void)
{
	return CVAR_GET_FLOAT("mp_allowmonsters") != 0;
}

void CHalfLifeMultiplay::GoToIntermission(void)
{
	if (g_fGameOver)
		return;

	if (g_pGameRules)
	{
		UTIL_LogPrintf("Team \"CT\" scored \"%i\" with \"%i\" players\n", g_pGameRules->m_iNumCTWins, g_pGameRules->m_iNumCT);
		UTIL_LogPrintf("Team \"TERRORIST\" scored \"%i\" with \"%i\" players\n", g_pGameRules->m_iNumTerroristWins, g_pGameRules->m_iNumTerrorist);
	}

	MESSAGE_BEGIN(MSG_ALL, SVC_INTERMISSION);
	MESSAGE_END();

	int time = (int)CVAR_GET_FLOAT("mp_chattime");

	if (time < 1)
		CVAR_SET_STRING("mp_chattime", "1");
	else if (time > MAX_INTERMISSION_TIME)
		CVAR_SET_STRING("mp_chattime", UTIL_dtos1(MAX_INTERMISSION_TIME));

	m_flIntermissionEndTime = gpGlobals->time + (int)mp_chattime.value;
	m_flIntermissionStartTime = gpGlobals->time;

	g_fGameOver = TRUE;
	m_iEndIntermissionButtonHit = FALSE;
	m_iSpawnPointCount_Terrorist = 0;
	m_iSpawnPointCount_CT = 0;
	m_bLevelInitialized = FALSE;
}

#define MAX_RULE_BUFFER 1024

typedef struct mapcycle_item_s
{
	struct mapcycle_item_s *next;
	char mapname[32];
	int minplayers, maxplayers;
	char rulebuffer[MAX_RULE_BUFFER];
}
mapcycle_item_t;

typedef struct mapcycle_s
{
	struct mapcycle_item_s *items;
	struct mapcycle_item_s *next_item;
}
mapcycle_t;

void DestroyMapCycle(mapcycle_t *cycle)
{
	mapcycle_item_t *n, *start;
	mapcycle_item_t *p = cycle->items;

	if (p)
	{
		start = p;
		p = p->next;

		while (p != start)
		{
			n = p->next;
			delete p;
			p = n;
		}

		delete cycle->items;
	}

	cycle->items = NULL;
	cycle->next_item = NULL;
}

static char com_token[1500];

char *COM_Parse(char *data)
{
	int len = 0, c;
	com_token[0] = '\0';

	if (!data)
		return NULL;

skipwhite:
	while ((c = *data) <= ' ')
	{
		if (c == 0)
			return NULL;

		data++;
	}

	if (c == '/' && data[1] == '/')
	{
		while (*data && *data != '\n')
			data++;

		goto skipwhite;
	}

	if (c == '\"')
	{
		data++;

		while (1)
		{
			c = *data++;

			if (c == '\"' || !c)
			{
				com_token[len] = '\0';
				return data;
			}

			com_token[len++] = c;
		}
	}

	if (c == '{' || c == '}'|| c == ')'|| c == '(' || c == '\'' || c == ',')
	{
		com_token[len++] = c;
		com_token[len] = '\0';
		return data + 1;
	}

	do
	{
		com_token[len] = c;
		data++;
		len++;
		c = *data;

		if (c == '{' || c == '}'|| c == ')'|| c == '(' || c == '\'' || c == ',')
			 break;
	}
	while (c > 32);

	com_token[len] = '\0';
	return data;
}

int COM_TokenWaiting(char *buffer)
{
	char *p = buffer;

	while (*p && *p != '\n')
	{
		if (!isspace(*p) || isalnum(*p))
			return 1;

		p++;
	}

	return 0;
}

int ReloadMapCycleFile(char *filename, mapcycle_t *cycle)
{
	int length;
	char *pFileList;
	char *aFileList = pFileList = (char *)LOAD_FILE_FOR_ME(filename, &length);
	mapcycle_item_s *item, *newlist = NULL, *next;

	if (pFileList && length)
	{
		while (1)
		{
			int hasbuffer = 0;
			char szBuffer[MAX_RULE_BUFFER];
			memset(szBuffer, 0, MAX_RULE_BUFFER);
			pFileList = COM_Parse(pFileList);

			if (strlen(com_token) == 0)
				break;

			char szMap[32];
			strcpy(szMap, com_token);

			if (COM_TokenWaiting(pFileList))
			{
				pFileList = COM_Parse(pFileList);

				if (strlen(com_token) != 0)
				{
					hasbuffer = 1;
					strcpy(szBuffer, com_token);
				}
			}

			if (IS_MAP_VALID(szMap))
			{
				item = new mapcycle_item_s;
				strcpy(item->mapname, szMap);

				item->minplayers = 0;
				item->maxplayers = 0;

				memset(item->rulebuffer, 0, MAX_RULE_BUFFER);

				if (hasbuffer)
				{
					char *s = g_engfuncs.pfnInfoKeyValue(szBuffer, "minplayers");

					if (s && s[0])
					{
						item->minplayers = atoi(s);
						item->minplayers = max(item->minplayers, 0);
						item->minplayers = min(item->minplayers, gpGlobals->maxClients);
					}

					s = g_engfuncs.pfnInfoKeyValue(szBuffer, "maxplayers");

					if (s && s[0])
					{
						item->maxplayers = atoi(s);
						item->maxplayers = max(item->maxplayers, 0);
						item->maxplayers = min(item->maxplayers, gpGlobals->maxClients);
					}

					g_engfuncs.pfnInfo_RemoveKey(szBuffer, "minplayers");
					g_engfuncs.pfnInfo_RemoveKey(szBuffer, "maxplayers");
					strcpy(item->rulebuffer, szBuffer);
				}

				item->next = cycle->items;
				cycle->items = item;
			}
			else
				ALERT(at_console, "Skipping %s from mapcycle, not a valid map\n", szMap);
		}

		FREE_FILE(aFileList);
	}

	item = cycle->items;

	while (item)
	{
		next = item->next;
		item->next = newlist;
		newlist = item;
		item = next;
	}

	cycle->items = newlist;
	item = cycle->items;

	if (!item)
		return 0;

	while (item->next)
		item = item->next;

	item->next = cycle->items;
	cycle->next_item = item->next;
	return 1;
}

int CountPlayers(void)
{
	int num = 0;

	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBaseEntity *pEnt = UTIL_PlayerByIndex(i);

		if (pEnt)
			num = num + 1;
	}

	return num;
}

void ExtractCommandString(char *s, char *szCommand)
{
	char pkey[512];
	char value[512];

	if (*s == '\\')
		s++;

	while (1)
	{
		char *o = pkey;

		while (*s != '\\')
		{
			if (!*s)
				return;

			*o++ = *s++;
		}

		*o = '\0';
		s++;
		o = value;

		while (*s != '\\' && *s)
		{
			if (!*s)
				return;

			*o++ = *s++;
		}

		*o = '\0';
		strcat(szCommand, pkey);

		if (strlen(value) != 0)
		{
			strcat(szCommand, " ");
			strcat(szCommand, value);
		}

		strcat(szCommand, "\n");

		if (!*s)
			return;

		s++;
	}
}

int GetMapCount(void)
{
	static mapcycle_t mapcycle;
	char *mapcfile = (char *)CVAR_GET_STRING("mapcyclefile");

	DestroyMapCycle(&mapcycle);
	ReloadMapCycleFile(mapcfile, &mapcycle);

	int num = 0;

	for (mapcycle_item_s *item = mapcycle.next_item; item->next != mapcycle.next_item; item = item->next)
		num = num + 1;

	return num;
}

void CHalfLifeMultiplay::Cmd_AddCondition(CBasePlayer *pPlayer)
{
	int argc = CMD_ARGC();
	if(argc < 2)
	{
		ClientPrint(pPlayer->pev, HUD_PRINTCONSOLE, "Usage: addcond <buff> [time]\n1. invulnerable\n2. critboost\n3. afterburn\n4. noclip\n");
		return;
	}
	int act = atoi(CMD_ARGV(1));
	float time = 60;
	if(argc > 2)
	{
		time = atof(CMD_ARGV(2));
	}
	switch(act)
	{
	case 1:
		pPlayer->Invulnerable_Add(time, 0);
		break;
	case 2:
		pPlayer->CritBoost_Add(time);
		break;
	case 3:
		pPlayer->AfterBurn_Add(time, 5, pPlayer->pev, pPlayer->m_pActiveItem ? pPlayer->m_pActiveItem->pev : pPlayer->pev);
		break;
	case 4:
		if(pPlayer->pev->movetype != MOVETYPE_NOCLIP)
			pPlayer->pev->movetype = MOVETYPE_NOCLIP;
		else
			pPlayer->pev->movetype = MOVETYPE_WALK;
		break;
	default:
		ClientPrint(pPlayer->pev, HUD_PRINTCONSOLE, "addcond: invalid command\n");
		break;
	}
}

void CHalfLifeMultiplay::DisplayMaps(CBasePlayer *pPlayer, int mapId)
{
	static mapcycle_t mapcycle;
	char *mapcfile = (char *)CVAR_GET_STRING("mapcyclefile");
	int index = 0, id = 0;
	char *pszNextMaps = NULL;

	DestroyMapCycle(&mapcycle);
	ReloadMapCycleFile(mapcfile, &mapcycle);

	for (mapcycle_item_s *item = mapcycle.next_item; item->next != mapcycle.next_item; item = item->next, index++)
	{
		id++;

		if (pPlayer)
		{
			if (m_rgiVotingMapCount[index] != 1)
				ClientPrint(pPlayer->pev, HUD_PRINTCONSOLE, "#Votes", UTIL_dtos1(index), item->mapname, UTIL_dtos2(m_rgiVotingMapCount[index]));
			else
				ClientPrint(pPlayer->pev, HUD_PRINTCONSOLE, "#Vote", UTIL_dtos1(index), item->mapname, UTIL_dtos2(1));
		}

		if (id == mapId)
			pszNextMaps = item->mapname;
	}

	if (!pszNextMaps || !mapId)
		return;

	if (strcmp(pszNextMaps, STRING(gpGlobals->mapname)))
	{
		CHANGE_LEVEL(pszNextMaps, NULL);
		return;
	}

	if (timelimit.value)
	{
		timelimit.value += 30;
		UTIL_ClientPrintAll(HUD_PRINTCENTER, "#Map_Vote_Extend");
	}

	ResetAllMapVotes();
}

void CHalfLifeMultiplay::ResetAllMapVotes(void)
{
	CBaseEntity *pEntity = NULL;

	while ((pEntity = UTIL_FindEntityByClassname(pEntity, "player")) != NULL)
	{
		if (FNullEnt(pEntity->edict()))
			break;

		CBasePlayer *pPlayer = GetClassPtr((CBasePlayer *)pEntity->pev);

		if (pPlayer->m_iTeam != TEAM_UNASSIGNED)
			pPlayer->m_iVotingMapId = 0;
	}

	for (int i = 0; i < MAX_MAPS; i++)
		m_rgiVotingMapCount[i] = 0;
}

void CHalfLifeMultiplay::ProcessMapVote(CBasePlayer *pPlayer, int mapId)
{
	CBaseEntity *pEntity = NULL;
	int playerCount = 0, count = 0;

	while ((pEntity = UTIL_FindEntityByClassname(pEntity, "player")) != NULL)
	{
		if (FNullEnt(pEntity->edict()))
			break;

		CBasePlayer *pPlayer = GetClassPtr((CBasePlayer *)pEntity->pev);

		if (pPlayer->m_iTeam != TEAM_UNASSIGNED)
		{
			playerCount++;

			if (pPlayer->m_iVotingMapId = mapId)
				count++;
		}
	}

	m_rgiVotingMapCount[mapId] = count;
	float radio = mapvoteratio.value;

	if (mapvoteratio.value > 1)
	{
		radio = 1;
		CVAR_SET_STRING("mp_mapvoteratio", "1.0");
	}
	else if (mapvoteratio.value < 0.35)
	{
		radio = 0.35;
		CVAR_SET_STRING("mp_mapvoteratio", "0.35");
	}

	int needCount;

	if (playerCount > 2)
		needCount = (int)(playerCount * radio + 0.5);
	else
		needCount = 2;

	if (count < needCount)
	{
		DisplayMaps(pPlayer, 0);
		ClientPrint(pPlayer->pev, HUD_PRINTCONSOLE, "#Game_required_votes", UTIL_dtos1(needCount));
	}
	else
		DisplayMaps(NULL, mapId);
}

void CHalfLifeMultiplay::ChangeLevel(void)
{
	static char szPreviousMapCycleFile[256];
	static mapcycle_t mapcycle;

	char szNextMap[32];
	char szFirstMapInList[32];
	char szCommands[1500];
	char szRules[1500];
	int minplayers = 0, maxplayers = 0;
	strcpy(szFirstMapInList, "hldm1");

	int curplayers;
	BOOL do_cycle = TRUE;
	char *mapcfile = (char *)CVAR_GET_STRING("mapcyclefile");

	szCommands[0] = '\0';
	szRules[0] = '\0';
	curplayers = CountPlayers();

	if (stricmp(mapcfile, szPreviousMapCycleFile))
	{
		strcpy(szPreviousMapCycleFile, mapcfile);
		DestroyMapCycle(&mapcycle);

		if (!ReloadMapCycleFile(mapcfile, &mapcycle) || !mapcycle.items)
		{
			ALERT(at_console, "Unable to load map cycle file %s\n", mapcfile);
			do_cycle = FALSE;
		}
	}

	if (do_cycle && mapcycle.items)
	{
		BOOL keeplooking = FALSE;
		BOOL found = FALSE;
		mapcycle_item_s *item;

		strcpy(szNextMap, STRING(gpGlobals->mapname));
		strcpy(szFirstMapInList, STRING(gpGlobals->mapname));

		for (item = mapcycle.next_item; item->next != mapcycle.next_item; item = item->next)
		{
			keeplooking = FALSE;

			if (item->minplayers)
			{
				if (curplayers >= item->minplayers)
				{
					found = TRUE;
					minplayers = item->minplayers;
				}
				else
					keeplooking = TRUE;
			}

			if (item->maxplayers)
			{
				if (curplayers <= item->maxplayers)
				{
					found = TRUE;
					maxplayers = item->maxplayers;
				}
				else
					keeplooking = TRUE;
			}

			if (keeplooking)
				continue;

			found = TRUE;
			break;
		}

		if (!found)
			item = mapcycle.next_item;

		mapcycle.next_item = item->next;
		strcpy(szNextMap, item->mapname);
		ExtractCommandString(item->rulebuffer, szCommands);
		strcpy(szRules, item->rulebuffer);
	}

	if (!IS_MAP_VALID(szNextMap))
		strcpy(szNextMap, szFirstMapInList);

	g_fGameOver = TRUE;
	ALERT(at_console, "CHANGE LEVEL: %s\n", szNextMap);

	if (minplayers || maxplayers)
		ALERT(at_console, "PLAYER COUNT:  min %i max %i current %i\n", minplayers, maxplayers, curplayers);

	if (strlen(szRules))
		ALERT(at_console, "RULES:  %s\n", szRules);

	CHANGE_LEVEL(szNextMap, NULL);

	if (strlen(szCommands))
		SERVER_COMMAND(szCommands);
}

#define MAX_MOTD_CHUNK 51
#define MAX_MOTD_LENGTH 2048

void CHalfLifeMultiplay::SendMOTDToClient(CBasePlayer *pPlayer)
{
	int length, char_count = 0;
	char *pFileList;
	char *aFileList = pFileList = (char *)LOAD_FILE_FOR_ME((char *)CVAR_GET_STRING("motdfile"), &length);

	//if(length < MAX_MOTD_LENGTH)
	//{
	//	SendMGUIPrint(pPlayer, MENUBUF_MOTD, aFileList);
	//}

	while (pFileList && *pFileList && char_count < MAX_MOTD_LENGTH)
	{
		char chunk[MAX_MOTD_CHUNK + 1];

		if (strlen(pFileList) < MAX_MOTD_CHUNK)
		{
			strcpy(chunk, pFileList);
		}
		else
		{
			strncpy(chunk, pFileList, MAX_MOTD_CHUNK);
			chunk[MAX_MOTD_CHUNK] = '\0';
		}

		char_count += strlen(chunk);

		if (char_count < MAX_MOTD_LENGTH)
			pFileList = aFileList + char_count;
		else
			*pFileList = '\0';

		MESSAGE_BEGIN(MSG_ONE, gmsgMOTD, NULL, pPlayer->edict());
		WRITE_BYTE(*pFileList ? FALSE : TRUE);
		WRITE_STRING(chunk);
		MESSAGE_END();
	}

	MESSAGE_BEGIN(MSG_ONE, gmsgServerName, NULL, pPlayer->edict());
	WRITE_STRING(CVAR_GET_STRING("hostname"));
	MESSAGE_END();

	if(length && aFileList && *aFileList)
	{
		FREE_FILE(aFileList);
	}
	//ShowMGUIMenu(pPlayer, MENU_INTRO, 0);
	ShowVGUIMenu(pPlayer, MENU_INTRO, 0, "");
}

void CHalfLifeMultiplay::ClientUserInfoChanged(CBasePlayer *pPlayer, char *infobuffer)
{
	pPlayer->SetPlayerModel();
	pPlayer->SetPrefsFromUserinfo(infobuffer);
}

BOOL CHalfLifeMultiplay::CPRoundEndCheck(BOOL bNeededPlayers)
{
	int action = END_NOTHING;

	if (m_bMapHasControlPoint)
	{
		if(CPCountPoints(TEAM_RED) >= m_ControlPoints.Count())
			action = m_iRedDominatedAction;
		if(CPCountPoints(TEAM_BLU) >= m_ControlPoints.Count())
			action = m_iBluDominatedAction;
	}

	switch (action)
	{
	case END_NOTHING:
		break;
	case END_DRAW:
		TerminateRound(m_iEndTime, WINSTATUS_DRAW);
		return TRUE;
	case END_RED_WIN:
		if (!bNeededPlayers)
		{
			m_iNumTerroristWins++;
			UpdateTeamScores();
		}
		TerminateRound(m_iEndTime, WINSTATUS_TERRORIST);
		return TRUE;
	case END_BLU_WIN:
		if (!bNeededPlayers)
		{
			m_iNumCTWins++;
			UpdateTeamScores();
		}
		TerminateRound(m_iEndTime, WINSTATUS_CT);
		return TRUE;
	case END_SUDDEN_DEATH:
		// TODO NYI
		break;
	}

	return FALSE;
}

int CHalfLifeMultiplay::CPCountPoints(int iTeam)
{
	int iCount = 0;
	
	for(int i = 0; i < m_ControlPoints.Count(); ++i)
	{
		if(iTeam == m_ControlPoints[i]->v.team)
			iCount ++;
	}
	return iCount;
}

extern int gmsgCPState;
extern int gmsgCPInit;

void CHalfLifeMultiplay::CPSendState(entvars_t *pevPoint)
{
	if(!pevPoint)
		return;

	CControlPoint *pPoint = (CControlPoint *)CBaseEntity::Instance(pevPoint);

	int i;

	for(i = 0; i < m_ControlPoints.Count(); ++i)
	{
		if(m_ControlPoints[i] == pevPoint->pContainingEntity)
			break;
	}

	if(i == m_ControlPoints.Count())
		return;

	MESSAGE_BEGIN(MSG_ALL, gmsgCPState);
	WRITE_BYTE(i);
	WRITE_BYTE(pPoint->m_bLocked);
	WRITE_BYTE(pPoint->m_bDisabled);
	WRITE_BYTE(pPoint->m_iState);
	WRITE_BYTE(pPoint->m_iCapTeam | (pPoint->pev->team << 2));
	WRITE_BYTE(pPoint->m_iCapPlayers);
	WRITE_COORD(pPoint->m_flProgress);
	WRITE_COORD(pPoint->m_flCapRate);
	MESSAGE_END();
}

void CHalfLifeMultiplay::CPSendState(void)
{
	for(int i = 0; i < m_ControlPoints.Count(); ++i)
	{
		CControlPoint *pPoint = (CControlPoint *)CBaseEntity::Instance(m_ControlPoints[i]);
		MESSAGE_BEGIN(MSG_ALL, gmsgCPState);		
		WRITE_BYTE(i);
		WRITE_BYTE(pPoint->m_bLocked);
		WRITE_BYTE(pPoint->m_bDisabled);
		WRITE_BYTE(pPoint->m_iState);
		WRITE_BYTE(pPoint->m_iCapTeam | (pPoint->pev->team << 2));
		WRITE_BYTE(pPoint->m_iCapPlayers);
		WRITE_COORD(pPoint->m_flProgress);
		WRITE_COORD(pPoint->m_flCapRate);
		MESSAGE_END();
	}	
}

void CHalfLifeMultiplay::CPSendState(CBasePlayer *pPlayer)
{
	for(int i = 0; i < m_ControlPoints.Count(); ++i)
	{
		CControlPoint *pPoint = (CControlPoint *)CBaseEntity::Instance(m_ControlPoints[i]);
		MESSAGE_BEGIN(MSG_ONE, gmsgCPState, NULL, pPlayer->pev);
		WRITE_BYTE(i);
		WRITE_BYTE(pPoint->m_bLocked);
		WRITE_BYTE(pPoint->m_bDisabled);
		WRITE_BYTE(pPoint->m_iState);
		WRITE_BYTE(pPoint->m_iCapTeam | (pPoint->pev->team << 2));
		WRITE_BYTE(pPoint->m_iCapPlayers);
		WRITE_COORD(pPoint->m_flProgress);
		WRITE_COORD(pPoint->m_flCapRate);
		MESSAGE_END();
	}
}

void CHalfLifeMultiplay::CPSendInit(CBasePlayer *pPlayer)
{
	if(!m_ControlPoints.Count()) return;

	int count = m_ControlPoints.Count();
	MESSAGE_BEGIN(MSG_ONE, gmsgCPInit, NULL, pPlayer->pev);
	WRITE_BYTE(count);
	for(int i = 0; i < count; ++i)
	{
		CControlPoint *pPoint = (CControlPoint *)CBaseEntity::Instance(m_ControlPoints[i]);
		WRITE_CHAR(pPoint->m_iHUDPosition);
		WRITE_STRING(pPoint->m_szName);
		WRITE_STRING(STRING(pPoint->pev->model));
	}
	MESSAGE_END();
}

void CHalfLifeMultiplay::CPResetAll(void)
{
	for(int i = 0; i < m_ControlPoints.Count(); ++i)
	{
		CControlPoint *pPoint = (CControlPoint *)CBaseEntity::Instance(m_ControlPoints[i]);
		pPoint->Restart();		
	}
}

BOOL CHalfLifeMultiplay::CPCheckOvertime(void)
{
	for(int i = 0; i < m_ControlPoints.Count(); ++i)
	{
		CControlPoint *pPoint = (CControlPoint *)CBaseEntity::Instance(m_ControlPoints[i]);
		if(pPoint->m_flProgress > 0 && pPoint->m_iTimeAdded > 0)
			return TRUE;
	}
	return FALSE;
}

extern int gmsgObjectMsg;

void CHalfLifeMultiplay::ObjectNotice(int iObjectIndex, int iObjectAction, int iKillerTeam, int iVictimTeam, CBasePlayer *plAssister[], int iAssister)
{
	MESSAGE_BEGIN(MSG_ALL, gmsgObjectMsg);
	WRITE_BYTE(iObjectIndex);
	WRITE_BYTE(iObjectAction);
	WRITE_BYTE(iKillerTeam);
	WRITE_BYTE(iVictimTeam);

	//display maximum to 5 assister in object notice
	if(iAssister > 5)
		iAssister = 5;

	WRITE_BYTE(iAssister);	
	int assister_index = 0;
	for(int i = 0; i < iAssister; i++)
	{
		assister_index = plAssister[i]->entindex();
		WRITE_BYTE(assister_index);
	}

	MESSAGE_END();
}

extern int gmsgRoundTime;

void CHalfLifeMultiplay::SyncRoundTimer(void)
{
	float time;
	float maxtime;

	if (IsMultiplayer())
		time = TimeRemaining();
	else
		time = 0;

	if (time < 0)
		time = 0;

	if (IsMultiplayer())
		maxtime = TimeTotal();
	else
		maxtime = 0;

	if (maxtime < 0)
		maxtime = 0;

	MESSAGE_BEGIN(MSG_ALL, gmsgRoundTime);
	WRITE_BYTE(m_iRoundStatus);
	WRITE_SHORT(time);
	WRITE_SHORT(maxtime);
	MESSAGE_END();
}

void CHalfLifeMultiplay::AnnounceRoundTime(void)
{
	if (gpGlobals->time <= m_flAnnounceRoundTime)
		return;

	m_flAnnounceRoundTime = gpGlobals->time;

	int iRemaining = TimeRemaining();
	if(m_iRoundStatus == ROUND_SETUP)
	{
		if(iRemaining == 60)
			UTIL_PlayMP3(NULL, "sound/CKF_III/ano/announcer_begins_60sec.mp3");
		else if(iRemaining == 30)
			UTIL_PlayMP3(NULL, "sound/CKF_III/ano/announcer_begins_30sec.mp3");
		else if(iRemaining == 20)
			UTIL_PlayMP3(NULL, "sound/CKF_III/ano/announcer_begins_20sec.mp3");
		else if(iRemaining == 10)
			UTIL_PlayMP3(NULL, "sound/CKF_III/ano/announcer_begins_10sec.mp3");
		else if(iRemaining == 5)
			UTIL_PlayMP3(NULL, "sound/CKF_III/ano/announcer_begins_5sec.mp3");
		else if(iRemaining == 4)
			UTIL_PlayMP3(NULL, "sound/CKF_III/ano/announcer_begins_4sec.mp3");
		else if(iRemaining == 3)
			UTIL_PlayMP3(NULL, "sound/CKF_III/ano/announcer_begins_3sec.mp3");
		else if(iRemaining == 2)
			UTIL_PlayMP3(NULL, "sound/CKF_III/ano/announcer_begins_2sec.mp3");
		else if(iRemaining == 1)
			UTIL_PlayMP3(NULL, "sound/CKF_III/ano/announcer_begins_1sec.mp3");
	}
	else if(m_iRoundStatus == ROUND_NORMAL)
	{
		if(iRemaining == 300)
			UTIL_PlayMP3(NULL, "sound/CKF_III/ano/announcer_ends_5mins.mp3");
		else if(iRemaining == 120)
			UTIL_PlayMP3(NULL, "sound/CKF_III/ano/announcer_ends_2mins.mp3");
		else if(iRemaining == 60)
			UTIL_PlayMP3(NULL, "sound/CKF_III/ano/announcer_ends_60sec.mp3");
		else if(iRemaining == 30)
			UTIL_PlayMP3(NULL, "sound/CKF_III/ano/announcer_ends_30sec.mp3");
		else if(iRemaining == 20)
			UTIL_PlayMP3(NULL, "sound/CKF_III/ano/announcer_ends_20sec.mp3");
		else if(iRemaining == 10)
			UTIL_PlayMP3(NULL, "sound/CKF_III/ano/announcer_ends_10sec.mp3");
		else if(iRemaining == 5)
			UTIL_PlayMP3(NULL, "sound/CKF_III/ano/announcer_ends_5sec.mp3");
		else if(iRemaining == 4)
			UTIL_PlayMP3(NULL, "sound/CKF_III/ano/announcer_ends_4sec.mp3");
		else if(iRemaining == 3)
			UTIL_PlayMP3(NULL, "sound/CKF_III/ano/announcer_ends_3sec.mp3");
		else if(iRemaining == 2)
			UTIL_PlayMP3(NULL, "sound/CKF_III/ano/announcer_ends_2sec.mp3");
		else if(iRemaining == 1)
			UTIL_PlayMP3(NULL, "sound/CKF_III/ano/announcer_ends_1sec.mp3");
	}
}

void CHalfLifeMultiplay::UpdateTimeLimit(void)
{
	MESSAGE_BEGIN(MSG_BROADCAST, gmsgTimeLimit);
	if(!timelimit.value)
		WRITE_SHORT(0);
	else
		WRITE_SHORT((int)(g_flTimeLimit - gpGlobals->time));
	MESSAGE_END();
}

BOOL CHalfLifeMultiplay::IsRoundSetup(void)
{
	return (m_iRoundStatus >= ROUND_WAIT && m_iRoundStatus <= ROUND_SETUP);
}