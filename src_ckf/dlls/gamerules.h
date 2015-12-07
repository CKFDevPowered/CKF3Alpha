#pragma once

#include "metarender.h"
#include <tier1\UtlVector.h>

class CBasePlayerItem;
class CBasePlayer;
class CItem;
class CBasePlayerAmmo;
class CBaseBuildable;//hz extern here
class CBuildSapper;//hz extern here

enum
{
	GR_NONE = 0,
	GR_WEAPON_RESPAWN_YES,
	GR_WEAPON_RESPAWN_NO,
	GR_AMMO_RESPAWN_YES,
	GR_AMMO_RESPAWN_NO,
	GR_ITEM_RESPAWN_YES,
	GR_ITEM_RESPAWN_NO,
	GR_PLR_DROP_GUN_ALL,
	GR_PLR_DROP_GUN_ACTIVE,
	GR_PLR_DROP_GUN_NO,
	GR_PLR_DROP_AMMO_ALL,
	GR_PLR_DROP_AMMO_ACTIVE,
	GR_PLR_DROP_AMMO_NO
};

enum
{
	ROUND_NORMAL = 0,
	ROUND_SETUP,
	ROUND_END,
	ROUND_OVERTIME,
	ROUND_WAIT
};

enum
{
	END_NOTHING = 0,
	END_DRAW,
	END_RED_WIN,
	END_BLU_WIN,	
	END_SUDDEN_DEATH
};

enum
{
	WINSTATUS_NONE = 0,
	WINSTATUS_TERRORIST,
	WINSTATUS_CT,
	WINSTATUS_DRAW
};

enum
{
	WINREASON_NONE = 0,
	WINREASON_ALL_POINTS_CAPTURED,
	WINREASON_OPPONENTS_DEAD,
	WINREASON_FLAG_CAPTURE_LIMIT,
	WINREASON_DEFEND_UNTIL_TIME_LIMIT,
	WINREASON_STALEMATE,
};

enum
{
	GR_NOTTEAMMATE = 0,
	GR_TEAMMATE,
	GR_ENEMY,
	GR_ALLY,
	GR_NEUTRAL
};

class CGameRules
{
public:
	virtual void RefreshSkillData(void);
	virtual void Think(void) = 0;
	virtual BOOL IsAllowedToSpawn(CBaseEntity *pEntity) = 0;
	virtual BOOL FShouldSwitchWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon) = 0;
	virtual BOOL GetNextBestWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon) = 0;
	virtual BOOL IsMultiplayer(void) = 0;
	virtual BOOL IsDeathmatch(void) = 0;
	virtual BOOL IsTeamplay(void) { return FALSE; }
	virtual BOOL IsCoOp(void) = 0;
	virtual const char *GetGameDescription(void) { return "Chicken Fortress 3"; }
	virtual BOOL ClientConnected(edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[128]) = 0;
	virtual void InitHUD(CBasePlayer *pl) = 0;
	virtual void ClientDisconnected(edict_t *pClient) = 0;
	virtual void UpdateGameMode(CBasePlayer *pPlayer) {}
	virtual float FlPlayerFallDamage(CBasePlayer *pPlayer) = 0;
	virtual BOOL FPlayerCanTakeDamage(CBasePlayer *pPlayer, CBaseEntity *pAttacker) { return TRUE; }
	virtual BOOL ShouldAutoAim(CBasePlayer *pPlayer, edict_t *target) { return TRUE; }
	virtual void PlayerSpawn(CBasePlayer *pPlayer) = 0;
	virtual void PlayerThink(CBasePlayer *pPlayer) = 0;
	virtual BOOL FPlayerCanRespawn(CBasePlayer *pPlayer) = 0;
	virtual float FlPlayerSpawnTime(CBasePlayer *pPlayer) = 0;
	virtual edict_t *GetPlayerSpawnSpot(CBasePlayer *pPlayer);
	virtual BOOL AllowAutoTargetCrosshair(void) { return TRUE; }
	virtual BOOL ClientCommand_DeadOrAlive(CBasePlayer *pPlayer, const char *pcmd) { return FALSE; }
	virtual BOOL ClientCommand(CBasePlayer *pPlayer, const char *pcmd) { return FALSE; }
	virtual void ClientUserInfoChanged(CBasePlayer *pPlayer, char *infobuffer) {}
	virtual float IPointsForKill(CBasePlayer *pAttacker, CBaseEntity *pKilled) = 0;
	virtual void PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor) = 0;
	virtual void DeathNotice(CBasePlayer *pVictim, entvars_t *pevKiller, entvars_t *pevInflictor, entvars_t *pevAssister) = 0;
	virtual BOOL CanHavePlayerItem(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon);
	virtual void PlayerGotWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon) = 0;
	virtual int WeaponShouldRespawn(CBasePlayerItem *pWeapon) = 0;
	virtual float FlWeaponRespawnTime(CBasePlayerItem *pWeapon) = 0;
	virtual float FlWeaponTryRespawn(CBasePlayerItem *pWeapon) = 0;
	virtual Vector VecWeaponRespawnSpot(CBasePlayerItem *pWeapon) = 0;
	virtual BOOL CanHaveItem(CBasePlayer *pPlayer, CItem *pItem) = 0;
	virtual void PlayerGotItem(CBasePlayer *pPlayer, CItem *pItem) = 0;
	virtual int ItemShouldRespawn(CItem *pItem) = 0;
	virtual float FlItemRespawnTime(CItem *pItem) = 0;
	virtual Vector VecItemRespawnSpot(CItem *pItem) = 0;
	virtual BOOL CanHaveAmmo(CBasePlayer *pPlayer, const char *pszAmmoName, int iMaxCarry);
	virtual void PlayerGotAmmo(CBasePlayer *pPlayer, char *szName, int iCount) = 0;
	virtual int AmmoShouldRespawn(CBasePlayerAmmo *pAmmo) = 0;
	virtual float FlAmmoRespawnTime(CBasePlayerAmmo *pAmmo) = 0;
	virtual Vector VecAmmoRespawnSpot(CBasePlayerAmmo *pAmmo) = 0;
	virtual float FlHealthChargerRechargeTime(void) = 0;
	virtual float FlHEVChargerRechargeTime(void) { return 0; }
	virtual int DeadPlayerWeapons(CBasePlayer *pPlayer) = 0;
	virtual int DeadPlayerAmmo(CBasePlayer *pPlayer) = 0;
	virtual const char *GetTeamID(CBaseEntity *pEntity) = 0;
	virtual int PlayerRelationship(CBaseEntity *pPlayer, CBaseEntity *pTarget) = 0;
	virtual int GetTeamIndex(const char *pTeamName) { return -1; }
	virtual const char *GetIndexedTeamName(int teamIndex) { return ""; }
	virtual BOOL IsValidTeam(const char *pTeamName) { return TRUE; }
	virtual void ChangePlayerTeam(CBasePlayer *pPlayer, const char *pTeamName, BOOL bKill, BOOL bGib) {}
	virtual const char *SetDefaultPlayerTeam(CBasePlayer *pPlayer) { return ""; }
	virtual BOOL PlayTextureSounds(void) { return TRUE; }
	virtual BOOL FAllowMonsters(void) = 0;
	virtual void EndMultiplayerGame(void) {}
	virtual void CheckMapConditions(void) {}
	//hz added
};

extern char *GetTeam(int team);
extern void Broadcast(const char *sentence);
extern CGameRules *InstallGameRules(void);

class CHalfLifeRules : public CGameRules
{
public:
	CHalfLifeRules(void);

public:
	virtual void Think(void);
	virtual BOOL IsAllowedToSpawn(CBaseEntity *pEntity);
	virtual BOOL FShouldSwitchWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon);
	virtual BOOL GetNextBestWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon);
	virtual BOOL IsMultiplayer(void);
	virtual BOOL IsDeathmatch(void);
	virtual BOOL IsCoOp(void);
	virtual BOOL ClientConnected(edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[128]);
	virtual void InitHUD(CBasePlayer *pl);
	virtual void ClientDisconnected(edict_t *pClient);
	virtual float FlPlayerFallDamage(CBasePlayer *pPlayer);
	virtual void PlayerSpawn(CBasePlayer *pPlayer);
	virtual void PlayerThink(CBasePlayer *pPlayer);
	virtual BOOL FPlayerCanRespawn(CBasePlayer *pPlayer);
	virtual float FlPlayerSpawnTime(CBasePlayer *pPlayer);
	virtual edict_t *GetPlayerSpawnSpot(CBasePlayer *pPlayer);
	virtual BOOL AllowAutoTargetCrosshair(void);
	virtual float IPointsForKill(CBasePlayer *pAttacker, CBaseEntity *pKilled);
	virtual void PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor);
	virtual void DeathNotice(CBasePlayer *pVictim, entvars_t *pevKiller, entvars_t *pevInflictor, entvars_t *pevAssister);
	virtual void PlayerGotWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon);
	virtual int WeaponShouldRespawn(CBasePlayerItem *pWeapon);
	virtual float FlWeaponRespawnTime(CBasePlayerItem *pWeapon);
	virtual float FlWeaponTryRespawn(CBasePlayerItem *pWeapon);
	virtual Vector VecWeaponRespawnSpot(CBasePlayerItem *pWeapon);
	virtual BOOL CanHaveItem(CBasePlayer *pPlayer, CItem *pItem);
	virtual void PlayerGotItem(CBasePlayer *pPlayer, CItem *pItem);
	virtual int ItemShouldRespawn(CItem *pItem);
	virtual float FlItemRespawnTime(CItem *pItem);
	virtual Vector VecItemRespawnSpot(CItem *pItem);
	virtual void PlayerGotAmmo(CBasePlayer *pPlayer, char *szName, int iCount);
	virtual int AmmoShouldRespawn(CBasePlayerAmmo *pAmmo);
	virtual float FlAmmoRespawnTime(CBasePlayerAmmo *pAmmo);
	virtual Vector VecAmmoRespawnSpot(CBasePlayerAmmo *pAmmo);
	virtual float FlHealthChargerRechargeTime(void);
	virtual int DeadPlayerWeapons(CBasePlayer *pPlayer);
	virtual int DeadPlayerAmmo(CBasePlayer *pPlayer);
	virtual BOOL FAllowMonsters(void);
	virtual const char *GetTeamID(CBaseEntity *pEntity) { return ""; }
	virtual int PlayerRelationship(CBaseEntity *pPlayer, CBaseEntity *pTarget);
};

#define MAX_MAPS 100
#define MAX_VIPQUEUES 5

#include "voice_gamemgr.h"

class CHalfLifeMultiplay : public CGameRules
{
public:
	CHalfLifeMultiplay(void);

public:
	virtual void Think(void);
	virtual void RefreshSkillData(void);
	virtual BOOL IsAllowedToSpawn(CBaseEntity *pEntity);
	virtual BOOL FShouldSwitchWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon);
	virtual BOOL GetNextBestWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pCurrentWeapon);
	virtual BOOL IsMultiplayer(void);
	virtual BOOL IsDeathmatch(void);
	virtual BOOL IsCoOp(void);
	virtual BOOL ClientConnected(edict_t *pEntity, const char *pszName, const char *pszAddress, char szRejectReason[128]);
	virtual void InitHUD(CBasePlayer *pl);
	virtual void ClientDisconnected(edict_t *pClient);
	virtual void UpdateGameMode(CBasePlayer *pPlayer);
	virtual float FlPlayerFallDamage(CBasePlayer *pPlayer);
	virtual BOOL FPlayerCanTakeDamage(CBasePlayer *pPlayer, CBaseEntity *pAttacker);
	virtual void PlayerSpawn(CBasePlayer *pPlayer);
	virtual void PlayerThink(CBasePlayer *pPlayer);
	virtual BOOL FPlayerCanRespawn(CBasePlayer *pPlayer);
	virtual float FlPlayerSpawnTime(CBasePlayer *pPlayer);
	virtual edict_t *GetPlayerSpawnSpot(CBasePlayer *pPlayer);
	virtual BOOL AllowAutoTargetCrosshair(void);
	virtual BOOL ClientCommand_DeadOrAlive(CBasePlayer *pPlayer, const char *pcmd);
	virtual BOOL ClientCommand(CBasePlayer *pPlayer, const char *pcmd);
	virtual void ClientUserInfoChanged(CBasePlayer *pPlayer, char *infobuffer);
	virtual float IPointsForKill(CBasePlayer *pAttacker, CBaseEntity *pKilled);
	virtual void PlayerKilled(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pInflictor);
	virtual void DeathNotice(CBasePlayer *pVictim, entvars_t *pKiller, entvars_t *pevInflictor, entvars_t *pevAssister);//Overloaded
	virtual BOOL CanHavePlayerItem(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon);
	virtual void PlayerGotWeapon(CBasePlayer *pPlayer, CBasePlayerItem *pWeapon);
	virtual int WeaponShouldRespawn(CBasePlayerItem *pWeapon);
	virtual float FlWeaponRespawnTime(CBasePlayerItem *pWeapon);
	virtual float FlWeaponTryRespawn(CBasePlayerItem *pWeapon);
	virtual Vector VecWeaponRespawnSpot(CBasePlayerItem *pWeapon);
	virtual BOOL CanHaveItem(CBasePlayer *pPlayer, CItem *pItem);
	virtual void PlayerGotItem(CBasePlayer *pPlayer, CItem *pItem);
	virtual int ItemShouldRespawn(CItem *pItem);
	virtual float FlItemRespawnTime(CItem *pItem);
	virtual Vector VecItemRespawnSpot(CItem *pItem);
	virtual void PlayerGotAmmo(CBasePlayer *pPlayer, char *szName, int iCount);
	virtual int AmmoShouldRespawn(CBasePlayerAmmo *pAmmo);
	virtual float FlAmmoRespawnTime(CBasePlayerAmmo *pAmmo);
	virtual Vector VecAmmoRespawnSpot(CBasePlayerAmmo *pAmmo);
	virtual float FlHealthChargerRechargeTime(void);
	virtual float FlHEVChargerRechargeTime(void);
	virtual int DeadPlayerWeapons(CBasePlayer *pPlayer);
	virtual int DeadPlayerAmmo(CBasePlayer *pPlayer);
	virtual const char *GetTeamID(CBaseEntity *pEntity) { return ""; }
	virtual int PlayerRelationship(CBaseEntity *pPlayer, CBaseEntity *pTarget);
	virtual BOOL PlayTextureSounds(void) { return FALSE; }
	virtual BOOL FAllowMonsters(void);
	virtual void EndMultiplayerGame(void) { GoToIntermission(); }
	virtual void CheckMapConditions(void);
	virtual void CleanUpMap(void);
	virtual void RestartRound(void);
	virtual void CheckWinConditions(void);
	virtual void RemoveGuns(void);
	virtual void ChangeLevel(void);
	virtual void GoToIntermission(void);

public:
	void SendMOTDToClient(CBasePlayer *pPlayer);
	void ReadMultiplayCvars(void);
	void InitializePlayerCounts(int &NumAliveTerrorist, int &NumAliveCT, int &NumDeadTerrorist, int &NumDeadCT);
	BOOL NeededPlayersCheck(BOOL &bNeededPlayers);
	BOOL TeamExterminationCheck(int NumAliveTerrorist, int NumAliveCT, int NumDeadTerrorist, int NumDeadCT, BOOL bNeededPlayers);
	void BalanceTeams(void);
	BOOL IsThereABomber(void);
	BOOL IsThereABomb(void);
	BOOL TeamFull(int team_id);
	BOOL TeamStacked(int newTeam_id, int curTeam_id);
	void CheckWaitPeriodExpired(void);
	void CheckFreezePeriodExpired(void);
	void CheckSetupPeriodExpired(void);
	void CheckEndPeriodExpired(void);
	void CheckRoundTimeExpired(void);
	void CheckLevelInitialized(void);
	void CheckRestartRound(void);
	BOOL CheckTimeLimit(void);
	BOOL CheckMaxRounds(void);
	BOOL CheckGameOver(void);
	BOOL CheckWinLimit(void);
	void CheckSharedConVars(void);
	void CheckGameCvar(void);
	void DisplayMaps(CBasePlayer *pPlayer, int mapId);
	void ResetAllMapVotes(void);
	void ProcessMapVote(CBasePlayer *pPlayer, int mapId);
	void UpdateTeamScores(void);
	void SwapAllPlayers(void);
	void TerminateRound(float tmDelay, int iWinStatus);
	float TimeRemaining(void){ return m_iRoundTimeMax - gpGlobals->time + m_flRoundTimer; }
	float TimeElapsed(void) { return gpGlobals->time - m_flRoundTimer; }
	float TimeTotal(void) { return m_iRoundTimeMax; }

public:
	BOOL CPRoundEndCheck(BOOL bNeededPlayers);
	int CPCountPoints(int iTeam);
	void CPSendState(void);
	void CPSendState(entvars_t *pevPoint);
	void CPSendState(CBasePlayer *pPlayer);
	void CPSendInit(CBasePlayer *pPlayer);
	void CPResetAll(void);
	void ObjectNotice(int iObjectIndex, int iObjectAction, int iKillerTeam, int iVictimTeam, CBasePlayer *plKiller);
	void ObjectNotice(int iObjectIndex, int iObjectAction, int iKillerTeam, int iVictimTeam, CBasePlayer *pevAssister[], int iAssister);//Multiple assister
	void SyncRoundTimer(void);
	void SetRoundStatus(int iStatus);
	void SetRoundStatus(int iStatus, float flMaxTime);
	void SapperKilled(CBuildSapper *pVictim, entvars_t *pevKiller, entvars_t *pevInflictor);
	void BuildKilled(CBaseBuildable *pVictim, entvars_t *pKiller, entvars_t *pInflictor);//WTF
	void BuildDeathNotice(CBaseEntity *pVictim, entvars_t *pevKiller, entvars_t *pevInflictor);
	void AnnounceRoundTime(void);
	BOOL CPCheckOvertime(void);
	void CalcPoints(CBasePlayer *pAttacker, entvars_t *pevInflictor, CBasePlayer *pKilled);
	void UpdateTimeLimit(void);
	void DominateNemesis(CBasePlayer *pKiller, CBasePlayer *pVictim);
	CBasePlayer *GetAssister(CBasePlayer *pKiller, CBasePlayer *pVictim);
	void Cmd_AddCondition(CBasePlayer *pPlayer);
	BOOL IsRoundSetup(void);
	BOOL SendWinStatus(void);

public:
	CVoiceGameMgr m_VoiceGameMgr;
	int m_iNumTerrorist;
	int m_iNumCT;
	int m_iNumSpawnableTerrorist;
	int m_iNumSpawnableCT;
	int m_iSpawnPointCount_Terrorist;
	int m_iSpawnPointCount_CT;
	int m_iRoundWinStatus;
	int m_iRoundStatus;//ckf
	int m_iNumCTWins;
	int m_iNumTerroristWins;
	bool m_bMapHasResupplyRoom;
	int m_iMapHasIntroCamera;
	int m_iNumConsecutiveCTLoses;
	int m_iNumConsecutiveTerroristLoses;
	float m_fMaxIdlePeriod;
	int m_iLimitTeam;
	BOOL m_bLevelInitialized;
	BOOL m_bRoundTerminating;
	BOOL m_bCompleteReset;
	int m_iUnBalancedRounds;
	int m_rgiVotingMapCount[MAX_MAPS];
	int m_iMaxRounds;
	int m_iTotalRoundsPlayed;
	int m_iWinLimit;
	int m_iAllowSpectators;
	CBasePlayer *m_pVIP;
	CBasePlayer *m_pVIPQueue[MAX_VIPQUEUES];
	float m_flIntermissionEndTime;
	float m_flIntermissionStartTime;
	int m_iEndIntermissionButtonHit;
	float m_tmNextPeriodicThink;
	BOOL m_bFirstConnected;
	float m_fRedWaveTime;
	float m_fBluWaveTime;
	float m_fRedWaveThink;
	float m_fBluWaveThink;
	float m_fRedMinRespawn;
	float m_fRedMaxRespawn;
	float m_fBluMinRespawn;
	float m_fBluMaxRespawn;
	float m_fRespawnPlayerFactor;
	int m_iRespawnPlayerNum;

	float m_flRoundTimer;//the round time which is displayed
	int m_iRoundTimeMax;//the max round time which is displayed
	float m_flAnnounceRoundTime;

	int m_iFreezeTime;
	int m_iSetupCondition;
	int m_iWaitTime;
	int m_iSetupTime;
	int m_iRoundTime;
	int m_iEndTime;
	int m_iRespawnDisabled;
	int m_iEndAction;
	//bool m_bTimerExpired;
	bool m_bFreezePeriod;
	float m_flFreezeTimer;
	//For CP
	bool m_bMapHasControlPoint;
	int m_iRedLocal;
	int m_iBluLocal;
	//for control points
	CUtlVector<edict_t *> m_ControlPoints;
	//For nobuild zone
	CUtlVector<edict_t *> m_NoBuildZone;
	//For light_shadow
	CUtlVector<shadow_manager_t> m_ShadowManager;
};

extern DLL_GLOBAL CHalfLifeMultiplay *g_pGameRules;