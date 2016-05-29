#ifndef PLAYER_H
#define PLAYER_H

#include "pm_materials.h"
#include "hintmessage.h"

#define EF_INVULNERABLE				(1<<8)//256
#define EF_CRITBOOST				(1<<9)
#define EF_AFTERBURN				(1<<10)
#define EF_SHADOW					(1<<11)

#define FL_LOCK_DUCK				(1<<17)
#define	FL_LOCK_JUMP				(1<<18)
#define FL_MULTIJUMP				(1<<19)

#define MESSAGEPART_START			0
#define MESSAGEPART_MID				1
#define MESSAGEPART_END				2

enum fx_e
{
	FX_STICKYTRAIL = 0,
	FX_ROCKETTRAIL,
	FX_GRENADETRAIL,
	FX_KILLTRAIL,
	FX_KILLTRAIL_INSTANT,
	FX_BOTTLEBREAK,
	FX_MULTIJUMPTRAIL,
	FX_EXPLOSION_MIDAIR,
	FX_EXPLOSION_WALL,
	FX_STICKYKILL,
	FX_COORDTRACER,
	FX_ATTACHTRACER,
	FX_MEDIBEAM,
	FX_KILLMEDIBEAM,
	FX_BLUEPRINT,
	FX_DISGUISESMOKE,
	FX_SPYWATCH,
	FX_FLAMETHROW,
	FX_CRITHIT,
	FX_MINICRITHIT,
	FX_HITDAMAGE,
	FX_ADDHEALTH,
	FX_ADDTIME,
	FX_ADDMETAL,
	FX_BURNINGPLAYER,
	FX_CLOAKBEGIN,
	FX_CLOAKSTOP,
	FX_DISGUISEHINT,
	FX_KILLALLTRAIL,
	FX_CRITPLAYERWEAPON,
	FX_INVULNPLAYER,
	FX_AIRBLAST
};

enum
{
	PV_iDesiredClass = 0,
	PV_iLimitTeams
};

#define MAX_DMGRECORD 100

#define CloakBegin_Duration 1.2
#define CloakStop_Duration 2.2

#define PLAYER_FATAL_FALL_SPEED (float)1200
#define PLAYER_MAX_SAFE_FALL_SPEED (float)600
#define DAMAGE_FOR_FALL_SPEED (float)100.0 / (PLAYER_FATAL_FALL_SPEED - PLAYER_MAX_SAFE_FALL_SPEED)
#define PLAYER_MIN_BOUNCE_SPEED (float)350
#define PLAYER_FALL_PUNCH_THRESHHOLD (float)300.0

#define PFLAG_ONLADDER (1<<0)
#define PFLAG_ONSWING (1<<0)
#define PFLAG_ONTRAIN (1<<1)
#define PFLAG_ONBARNACLE (1<<2)
#define PFLAG_DUCKING (1<<3)
#define PFLAG_USING (1<<4)
#define PFLAG_OBSERVER (1<<5)
#define PFLAG_MULTIJUMP (1<<6)

#define TRAIN_ACTIVE 0x80 
#define TRAIN_NEW 0xc0
#define TRAIN_OFF 0x00
#define TRAIN_NEUTRAL 0x01
#define TRAIN_SLOW 0x02
#define TRAIN_MEDIUM 0x03
#define TRAIN_FAST 0x04 
#define TRAIN_BACK 0x05

#define DHF_ROUND_STARTED 0x2
#define DHF_HOSTAGE_SEE 0x4
#define DHF_HOSTAGE_FOLLOW 0x8
#define DHF_HOSTAGE_FOLLOWED 0x10
#define DHF_HOSTAGE_WARRING 0x20
#define DHF_HOSTAGE_KILLED 0x40
#define DHF_SPOTTED_FRIEND 0x80
#define DHF_SPOTTED_ENEMY 0x100
#define DHF_TEAMMATE_INJURE 0x200
#define DHF_TEAMMATE_WARRING 0x400
#define DHF_KILLEDENEMY 0x800
#define DHF_BOMBGUY 0x1000
#define DHF_AMMO_NOTENOUGH 0x8000
#define DHF_ENTERZONE_BOMB 0x10000
#define DHF_ENTERZONE_RESCUE 0x20000
#define DHF_ENTERZONE_ESCAPE 0x40000
#define DHF_ENTERZONE_VIPSAFETY 0x80000
#define DHF_BUY_NIGHTVISION 0x100000
#define DHF_HOSTAGE_CANTMOVE 0x200000

typedef enum
{
	JOINED,
	SHOWLTEXT,
	READINGLTEXT,
	SHOWTEAMSELECT,
	PICKINGTEAM,
	GETINTOGAME
}
JoinState;

#define MAPZONE_RESUPPLYROOM (1<<0)
#define MAPZONE_CONTROLPOINT (1<<1)
#define MAPZONE_NOBUILDABLE (1<<5)
#define MAPZONE_RESCUE (1<<2)
#define MAPZONE_ESCAPE (1<<3)
#define MAPZONE_VIPSAFETY (1<<4)

#define IGNOREMSG_NONE 0
#define IGNOREMSG_ENEMY 1
#define IGNOREMSG_TEAM 2

#define HUDMENU_DISGUISE 1
#define HUDMENU_BUILD 2
#define HUDMENU_DEMOLISH 3

//#define Menu_Radio1 11
//#define Menu_Radio2 12
//#define Menu_Radio3 13

#define CSUITPLAYLIST 4

#define SUIT_GROUP TRUE
#define SUIT_SENTENCE FALSE

#define SUIT_REPEAT_OK 0
#define SUIT_NEXT_IN_30SEC 30
#define SUIT_NEXT_IN_1MIN 60
#define SUIT_NEXT_IN_5MIN 300
#define SUIT_NEXT_IN_10MIN 600
#define SUIT_NEXT_IN_30MIN 1800
#define SUIT_NEXT_IN_1HOUR 3600

#define CSUITNOREPEAT 32

#define TEAM_NAME_LENGTH 16

#define CLOAK_NO 0
#define CLOAK_BEGIN 1
#define CLOAK_YES 2
#define CLOAK_STOP 3

#define DISGUISE_NO 0
#define DISGUISE_YES 1

enum playerstats_e
{
	STATS_CLEAR = 0,
	STATS_KILL,
	STATS_DEATH,
	STATS_ASSIST,
	STATS_DEMOLISH,
	STATS_CAPTURE,
	STATS_DEFENCE,
	STATS_DOMINATE,
	STATS_REVENGE,
	STATS_UBERCHARGE,
	STATS_HEADSHOT,
	STATS_TELEPORT,
	STATS_HEALING,
	STATS_BACKSTAB,
	STATS_BONUS,
	STATS_MAX_NUM
};

extern int gmsgFloatText;
extern int gmsgDrawFX;
extern int gmsgResetHUD;
extern int gmsgScoreInfo;

typedef enum
{
	PLAYER_IDLE,
	PLAYER_WALK,
	PLAYER_JUMP,
	PLAYER_SUPERJUMP,
	PLAYER_DIE,
	PLAYER_ATTACK1,
	PLAYER_ATTACK1_RIGHT,
	PLAYER_SMALL_FLINCH,
	PLAYER_LARGE_FLINCH,
	PLAYER_RELOAD,
	PLAYER_HOLDBOMB
}
PLAYER_ANIM;

#define MAX_ID_RANGE 2048
#define MAX_SPECTATOR_ID_RANGE 8192
#define SBAR_STRING_SIZE 128

enum sbar_rlation
{
	SBAR_RLATION_TEAMMATE = 1,
	SBAR_RLATION_ENEMY,
	SBAR_RLATION_HOSTAGE
};

enum sbar_data
{
	SBAR_ID_TARGETRLATION = 1,
	SBAR_ID_TARGETNAME,
	SBAR_ID_TARGETHEALTH,
	SBAR_END
};

#define CHAT_INTERVAL 1.0

class CBaseBuildable;

typedef struct
{
	int iKill;
	int iDeath;
	int iAssist;
	int iDemolish;
	int iCapture;
	int iDefence;
	int iDominate;
	int iRevenge;
	int iUbercharge;
	int iHeadshot;
	int iTeleport;
	int iHealing;
	int iBackstab;
	int iBonus;
}stats_t;

class CPlayerCondition
{
public:
	void Init(CBasePlayer *pPlayer);
	void Think(void);
	void Remove(void);
	virtual void OnThink(void){};
	virtual void OnRemove(void){};
public:
	int m_iStatus;
	float m_flDie;
	float m_flNextThink;
	float m_flNextEffect;
	CBasePlayer *m_pPlayer;
};

class CCondInvulnerable : public CPlayerCondition
{
public:
	void AddToPlayer(float flDuration, qboolean bFlash);
	virtual void OnThink(void);
	virtual void OnRemove(void);
public:
};

class CCondCritBoost : public CPlayerCondition
{
public:
	void AddToPlayer(float flDuration);
	virtual void OnThink(void);
	virtual void OnRemove(void);
public:
	float m_flNextEffect;
};

class CCondAfterBurn : public CPlayerCondition
{
public:
	void AddToPlayer(float flDuration, int iDamage, entvars_t *pevIgniter, entvars_t *pevInflictor);
	virtual void OnThink(void);
	virtual void OnRemove(void);
public:
	int m_iDamage;
	float m_flNextEffect;
	entvars_t *m_pevIgniter;
	entvars_t *m_pevInflictor;
};

typedef struct
{
	CCondInvulnerable Invulnerable;
	CCondCritBoost CritBoost;
	CCondAfterBurn AfterBurn;
}cond_manager_t;

typedef struct
{
	entvars_t *pevAttacker;
	int iDamage;
	float flTime;		
}dmgrecord_t;

typedef struct
{
	entvars_t *pevAttacker;
	int iDamage;
	int iCrit;	
}hitdamage_t;

class CBasePlayer : public CBaseMonster
{
public:
	CBasePlayer(void)
	{
		m_fKnownItem = FALSE;
		m_iMapZone = 0;
		m_iClientMapZone = 0;
		m_bIsRespawning = false;
		m_bCritBuff = false;
		m_bShouldClearBuild = false;
		memset(&m_Dominate, 0, sizeof(m_Dominate));
		m_iDefaultFOV = 90;
		m_pBuildable[0] = NULL;
		m_pBuildable[1] = NULL;
		m_pBuildable[2] = NULL;
		m_pBuildable[3] = NULL;

		m_flHitDamageTimer = 0;;
		m_flCritHitTimer = 0;

		Condition_Init();
	}

public:
	virtual void Spawn(void);
	virtual void Precache(void);
	virtual void Restart(void);
	virtual int Save(CSave &save);
	virtual int Restore(CRestore &restore);
	virtual int ObjectCaps(void) { return CBaseMonster::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	virtual int Classify(void);
	virtual void TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType, int iCrit);
	virtual int TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType, int iCrit);
	virtual int TakeHealth(float flHealth, int bitsDamageType);
	virtual void Killed(entvars_t *pevAttacker, int iGib);
	virtual void AddPoints(float score, BOOL bAllowNegativeScore);
	virtual void AddPointsToTeam(int score, BOOL bAllowNegativeScore);
	virtual BOOL AddPlayerItem(CBasePlayerItem *pItem);
	virtual BOOL RemovePlayerItem(CBasePlayerItem *pItem);
	virtual int GiveAmmo(int iAmount, char *szName, int iMax);
	virtual void StartSneaking(void) { m_tSneaking = gpGlobals->time - 1; }
	virtual void StopSneaking(void) { m_tSneaking = gpGlobals->time + 30; }
	virtual BOOL IsSneaking(void) { return m_tSneaking <= gpGlobals->time; }
	virtual BOOL IsAlive(void) { return pev->deadflag == DEAD_NO && pev->health > 0; }
	virtual BOOL IsPlayer(void) { return TRUE; }
	virtual BOOL IsNetClient(void) { return TRUE; }
	virtual const char *TeamID(void);
	virtual BOOL FBecomeProne(void);
	virtual Vector BodyTarget(const Vector &posSrc) { return Center() + pev->view_ofs * RANDOM_FLOAT(0.5, 1.1); }
	virtual int Illumination(void);
	virtual BOOL ShouldFadeOnDeath(void) { return FALSE; }
	virtual void ResetMaxSpeed(void);
	virtual void Jump(void);
	virtual void Duck(void);
	virtual void PreThink(void);
	virtual void PostThink(void);
	virtual Vector GetGunPosition(void);
	virtual void UpdateClientData(void);
	virtual void ImpulseCommands(void);
	//ckf3 hack
	virtual int FShouldCollide(CBaseEntity *pHit);
	virtual int GetCriticalHit(void);
public:
	void Pain(int hitgroup);
	void RenewItems(void);
	void PackDeadPlayerItems(void);
	void RemoveAllItems(BOOL removeSuit);
	void SwitchTeam(void);
	BOOL SwitchWeapon(CBasePlayerItem *pWeapon);
	BOOL IsOnLadder(void);
	void UpdatePlayerSound(void);
	void DeathSound(void);
	void SetAnimation(PLAYER_ANIM playerAnim);
	void SetWeaponAnimType(const char *szExtention);
	void CheatImpulseCommands(int iImpulse);
	void StartDeathCam(void);
	void StartObserver(Vector vecPosition, Vector vecViewAngle);
	CBaseEntity *Observer_IsValidTarget(int iTarget, bool bOnlyTeam);
	void Observer_FindNextPlayer(bool bReverse, char *name = NULL);
	void Observer_HandleButtons(void);
	void Observer_SetMode(int iMode);
	void Observer_CheckTarget(void);
	void Observer_CheckProperties(void);
	int IsObserver(void) { return pev->iuser1; }
	void DropPlayerItem(const char *pszItemName);
	void DropPrimary(void);
	void DropPistol(void);
	int GetPrimary(void);
	int GetSecondary(void);
	void ThrowWeapon(char *pszWeaponName);
	BOOL HasPlayerItem(CBasePlayerItem *pCheckItem);
	BOOL HasNamedPlayerItem(const char *pszItemName);
	BOOL HasWeapons(void);
	void SelectPrevItem(int iItem);
	void SelectNextItem(int iItem);
	void SelectLastItem(void);
	void SwitchSlotWeapon(int iSlot);
	void SelectItem(const char *pstr);
	void ItemPreFrame(void);
	void ItemPostFrame(void);
	void GiveNamedItem(const char *szName);
	void EnableControl(BOOL fControl);
	void SendAmmoUpdate(void);

	void WaterMove(void);
	void EXPORT PlayerDeathThink(void);
	void PlayerUse(void);
	void CheckSuitUpdate(void);
	void SetSuitUpdate(char *name, int fgroup, int iNoRepeat);
	void UpdateGeigerCounter(void);
	void CheckTimeBasedDamage(void);
	void BarnacleVictimBitten(entvars_t *pevBarnacle);
	void BarnacleVictimReleased(void);
	static int GetAmmoIndex(const char *psz);
	int AmmoInventory(int iAmmoIndex);
	void ResetAutoaim(void);
	Vector GetAutoaimVector(float flDelta);
	Vector AutoaimDeflection(Vector &vecSrc, float flDist, float flDelta);
	void ForceClientDllUpdate(void);
	void SetCustomDecalFrames(int nFrames);
	int GetCustomDecalFrames(void);
	void SetProgressBarTime(int iTime);
	void SetPlayerModel();//(BOOL bIsBombGuy);
	void Radio(const char *msg_id, const char *msg_verbose);
	void GiveDefaultItems(void);
	void SetBombIcon(BOOL bFlash);
	void SetScoreboardAttributes(CBasePlayer *pPlayer = NULL);
	int IsBombGuy(void);
	BOOL ShouldDoLargeFlinch(int hitgroup, int weaponId);
	BOOL IsArmored(int hitgroup);
	BOOL HintMessage(const char *message, BOOL bOnlyDead = FALSE, BOOL bImportant = FALSE);
	void AddAccount(int iAmount, BOOL flash);
	void SyncRoundTimer(void);
	void ResetMenu(void);
	void JoiningThink(void);
	void ResetStamina(void);
	void Disappear(void);
	void RemoveLevelText(void);
	void MoveToNextIntroCamera(void);
	void RoundRespawn(void);
	void SpawnClientSideCorpse(void);
	void SetPrefsFromUserinfo(char *infobuffer);
	void HostageUsed(void);
	void StudioEstimateGait(void);
	void CalculatePitchBlend(void);
	void CalculateYawBlend(void);
	void StudioProcessGait(void);
	void HandleSignals(void);
	//void UpdateStatusBar(void);
	void CheckPowerups(void);

	int GetBoostMaxHealth(void);
	int GetCriticalFire(int iType, unsigned int iRandSeed);
	void CritChance_Think(void);
	void CritBuff_Think(void);
	void Respawn_Think(void);
	void Respawn_Start(void);
	float GetPlayerMaxSpeed(void);
	void Cloak_Begin(void);
	void Cloak_Stop(void);
	void Cloak_Think(void);
	BOOL PlayerCanAttack(void);	
	void SendCriticalHit(BOOL bShowEffects, BOOL bShowSound);
	void SendMiniCritHit(BOOL bShowEffects, BOOL bShowSound);
	void SendAddHealth(int iHeal);
	void SendAddTime(int iTime);
	void SendAddMetal(int iMetal);
	void SendHitDamage(entvars_t *pevVictim, int iDamage, int iCrit);
	void ClearSticky(void);

	void Invulnerable_Add(float flDuration, qboolean bFlash);
	void CritBoost_Add(float flDuration);
	void AfterBurn_Add(float flDuration, int iDamage, entvars_t *pevIgniter, entvars_t *pevInflictor);

	void Condition_Init(void);
	void Condition_Think(void);
	void Condition_Clear(void);

	void Health_Think(void);
	void Uber_Think(void);
	BOOL ResupplyCase(void);
	float GetKnockBack(void);
	float GetPunch(void);
	void PostDeath(void);
	float GetCapRate(void);
	void DmgRecord_Add(CBasePlayer *pAttacker, int iDamage);
	int DmgRecord_Get(CBasePlayer *pAttacker, float flBefore);
	void DmgRecord_Clear(void);
	int DmgRecord_SendHitDamage(CBasePlayer *pAttacker);
	bool CanCapture(void);
	void Disguise_Start(int iTeam, int iClass);
	void Disguise_Weapon(void);
	void Disguise_Think(void);
	void Disguise_Stop(void);
	void SetDisguiseAnimation(PLAYER_ANIM playerAnim);
	void Build_Start(int iBuildClass);
	void Build_Deploy(void);
	BOOL Build_PreDeploy(Vector &vecSrc);
	void Build_UpdateHUD(void);
	void Build_DestroyAll(void);
	BOOL Build_PreUndeploy(void);
	void Build_Undeploy(void);
	BOOL EatAmmoBox(float flGivePercent);
	BOOL EatAmmoBox(float flAmmoPercent, float flMetalPercent, float flCloakPercent);
	void PlayerDisconnect(void);
	void KnockBack(Vector vecDir, float flForce);
	int AddPlayerMetal(int iMetalAdd);
	void Build_Demolish(int iBuildClass);
	void SendStatsInfo(int iType);
	void SendScoreInfo(void);
	int Dominate_Count(void);
	int PM_NoCollision(CBaseEntity *pEntity);
	void SendWeaponUpdate(CBasePlayerItem *pItem);
	bool IsObservingPlayer(CBasePlayer *pTarget);
	void SetObserverAutoDirector(bool bState);
	bool CanSwitchObserverModes(void);
	void SendSpecHealth(bool bShowEntIndex, entvars_t *pevInflictor);
	void UpdateDominate(void);
	void ClearDominates(void);
	void AddHitDamage(entvars_t *pevAttacker, int iDamage, int iCrit);
	void FlushHitDamage(void);
	void ClearEffects(void);

public:
	static TYPEDESCRIPTION m_playerSaveData[];

public:
	int random_seed;
	EHANDLE m_hObserverTarget;
	float m_flNextObserverInput;
	int m_iObserverWeapon;
	int m_iObserverC4State;
	bool m_bObserverHasDefuser;
	int m_iObserverLastMode;
	float m_flFlinchTime;
	bool m_bTakenHighDamage;
	float m_flVelocityModifier;
	float m_flEjectBrass;
	int m_iKevlar;
	bool m_bNotKilled;
	int m_iTeam;
	int m_iAccount;
	bool m_bHasPrimaryWeapon;
	float m_flDeathThrowTime;
	int m_iThrowDirection;
	float m_flLastTalk;
	bool m_bJustConnected;
	JoinState m_iJoiningState;
	CBaseEntity *m_pIntroCamera;
	float m_fIntroCamTime;
	float m_fLastMovement;
	bool m_bMissionBriefing;
	bool m_bTeamChanged;
	int m_iTeamKills;
	int m_iIgnoreMessage;
	bool m_bHasNightVision;
	bool m_bNightVisionOn;
	float m_flIdleCheckTime;
	float m_flNextRadioTime;
	int m_iRadioLeft;
	bool m_bIgnoreRadio;
	bool m_bIsBombGuy;
	bool m_bHasDefuseKit;
	bool m_bInjuredBlast;
	Vector m_vecInjuredLOS;
	bool m_bInjuredExplosion;
	CHintMessageQueue m_hintMessageQueue;
	int m_flDisplayHistory;
	int m_iMenu;
	int m_iHudMenu;
	float m_flUpdateSignalTime;
	int m_iVotingKikedPlayerId;
	float m_flNextVoteTime;
	bool m_bJustKilledTeammate;
	int m_iHostagesKilled;
	int m_iVotingMapId;
	bool m_bAllowAttack;
	float m_flLastAttackTime;
	bool m_bNameChanged;
	char m_szNewName[32];
	float m_flHandleSignalDelay;
	int m_iMapZone;
	int m_iClientMapZone;
	int m_iPlayerSound;
	int m_iTargetVolume;
	int m_iWeaponVolume;
	int m_iExtraSoundTypes;
	int m_iWeaponFlash;
	float m_flStopExtraSoundTime;
	int m_iClientCharge;
	int m_afButtonLast;
	int m_afButtonPressed;
	int m_afButtonReleased;
	edict_t *m_pentSndLast;
	float m_flSndRoomtype;
	float m_flSndRange;
	float m_flFallVelocity;
	int m_rgItems[MAX_ITEMS];
	int m_fKnownItem;
	int m_fNewAmmo;
	unsigned int m_afPhysicsFlags;
	float m_fNextSuicideTime;
	float m_flTimeStepSound;
	float m_flTimeWeaponIdle;
	float m_flSwimTime;
	float m_flDuckTime;
	float m_flWallJumpTime;
	float m_flSuitUpdate;
	int m_rgSuitPlayList[CSUITPLAYLIST];
	int m_iSuitPlayNext;
	int m_rgiSuitNoRepeat[CSUITNOREPEAT];
	float m_rgflSuitNoRepeatTime[CSUITNOREPEAT];
	int m_lastDamageAmount;
	float m_tbdPrev;
	float m_flgeigerRange;
	float m_flgeigerDelay;
	int m_igeigerRangePrev;
	int m_iStepLeft;
	char m_szTextureName[CBTEXTURENAMEMAX];
	char m_chTextureType;
	int m_idrowndmg;
	int m_idrownrestored;
	int m_bitsHUDDamage;
	BOOL m_fInitHUD;
	BOOL m_fGameHUDInitialized;
	int m_iTrain;
	BOOL m_fWeapon;
	EHANDLE m_pTank;
	float m_fDeadTime;
	BOOL m_fNoPlayerSound;
	float m_tSneaking;
	int m_iUpdateTime;
	int m_iClientHealth;
	int m_iClientBattery;
	int m_iHideHUD;
	int m_iClientHideHUD;
	int m_iFOV;
	int m_iClientFOV;
	int m_iNumSpawns;
	CBaseEntity *m_pLinkedEntity;
	CBasePlayerItem *m_rgpPlayerItems[MAX_ITEM_TYPES];
	CBasePlayerItem *m_pActiveItem;
	CBasePlayerItem *m_pClientActiveItem;
	CBasePlayerItem *m_pLastItem;
	int m_rgAmmo[MAX_AMMO_SLOTS];
	int m_rgAmmoLast[MAX_AMMO_SLOTS];
	Vector m_vecAutoAim;
	BOOL m_fOnTarget;
	int m_izSBarState[SBAR_END];
	float m_flStatusBarDisappearDelay;
	int m_lastx, m_lasty;
	int m_nCustomSprayFrames;
	float m_flNextDecalTime;
	//char m_szTeamName[TEAM_NAME_LENGTH];
	char m_szAnimExtention[32];
	int m_iGaitSequence;
	float m_flGaitFrame;
	float m_flGaitYaw;
	Vector m_vecPrevGaitOrigin;
	float m_flPitch;
	float m_flYaw;
	float m_flGaitMovement;
	int m_iAutoWepSwitch;
	BOOL m_bVGUIMenus;
	BOOL m_bShowHints;
	float m_flYawModifier;
	bool m_bObserverAutoDirector;
	bool m_canSwitchObserverModes;;
	bool m_bWasFollowing;
	float m_flNextFollowTime;
	//ckf
	int m_iClass;
	int m_iNewClass;
	bool m_bIsRespawning;
	bool m_bCritBuff;
	float m_fCritChance;
	float m_fCritBuffTimer;
	float m_fCritChanceTimer;
	int m_iDmgDone_Recent;
	float m_fDmgDone_Decesase;
	float m_fRespawnTimer;
	float m_fRespawnCounter;
	int m_iRocketJump;
	int m_iMultiJumpCurrent;
	int m_iMultiJumpMax;
	bool m_bMultiJump;
	//Cloak Field
	int m_iCloak;
	float m_fCloakTimer;
	float m_flCloakEnergy;
	float m_flCloakEnergyTimer;
	unsigned long m_ulModelIndexPlayer;
	float m_flHitDamageTimer;
	float m_flCritHitTimer;
	bool m_bCritKilled;
	bool m_bLastHitCrit;
	bool m_bBackStabKilled;
	bool m_bLastHitBackStab;
	bool m_bHeadShotKilled;
	float m_fUbercharge;
	int m_iUbercharge;
	float m_flUberThink;
	float m_flLastDamagedTime;
	float m_flHealthReduce;
	float m_flHealthRegen;
	cond_manager_t m_Cond;
	float m_flResupplyCase;
	edict_t *m_pentControlPoint;
	//DmageRecord
	CUtlVector<dmgrecord_t> m_DmgRecord;
	CUtlVector<hitdamage_t> m_HitDamage;
	//Disguise Field
	/*struct
	{
		int iStatus;
		int iBegin;
		float flTime;
		int iNewTeam;
		int iNewClass;
		int iTeam;
		int iClass;
		int iHealth;
		int iMaxHealth;
		int iWeaponModel;
		int iWeaponID;
		int iWeaponBody;
		float flHealthReduce;
		char szAnimExt[32];
		int iSequence;
		int iStandSequence;
		int iMaxSpeed;
		CBasePlayer *pTargetPlayer;
		int iTargetPlayer
	}m_Disguise;*/
	int m_iDisguise;
	bool m_bDisguiseStart;
	float m_flDisguiseTimer;
	int m_iDisguiseNewTeam;
	int m_iDisguiseNewClass;
	int m_iDisguiseTeam;
	int m_iDisguiseClass;
	int m_iDisguiseHealth;
	int m_iDisguiseMaxHealth;
	int m_iDisguiseWeapon;
	int m_iDisguiseWeaponID;
	int m_iDisguiseWeaponBody;
	float m_flDisguiseHealthReduce;
	char m_szDisguiseAnimExtention[32];
	int m_iDisguiseSequence;
	int m_iDisguiseStandSequence;
	int m_iDisguiseMaxSpeed;
	CBasePlayer *m_pDisguiseTarget;
	int m_iDisguiseTarget;
	//Status Bar Field
	float m_flNextSBarUpdateTime;
	int m_iClientSBarFlags;
	int m_iClientSBarEntIndex;
	int m_iClientSBarEntData;
	bool m_bShouldClearBuild;
	int m_iDefaultFOV;
	stats_t m_Stats;
	int m_iDominates;
	int m_Dominate[33];
	//For Medic
	float m_flHealAmount;
	CBasePlayer *m_pHealer;
	int m_iHealerCount;
	//For Engineer
	int m_iMetal;
	int m_iCarryBluePrint;
	int m_iBluePrintYaw;
	CBaseBuildable *m_pCarryBuild;
	CBaseBuildable *m_pBuildable[4];
	bool m_bAutoReload;
};

#define UC_INVULNERABLE (1<<0)
#define UC_CRITICAL (1<<1)

#define AUTOAIM_2DEGREES 0.0348994967025
#define AUTOAIM_5DEGREES 0.08715574274766
#define AUTOAIM_8DEGREES 0.1391731009601
#define AUTOAIM_10DEGREES 0.1736481776669

extern int gmsgHudText;
extern int gmsgShowMenu;
extern int gmsgVGUIMenu;
extern int gmsgMGUIMenu;
extern int gmsgMGUIPrint;
extern int gmsgHudMenu;
extern int gmsgRespawnTime;
extern int gmsgShake;
extern DLL_GLOBAL short g_sModelIndexPlayerClass[9];

extern void ShowHudMenu(CBasePlayer *pPlayer, int type, int keys, int skiplocal);
extern void ShowMGUIMenu(CBasePlayer *pPlayer, int type, int keys);
extern void SendMGUIPrint(CBasePlayer *pPlayer, int printbuf, const char *text);
extern void SendRespawnTime(CBasePlayer *pPlayer, int time);

extern BOOL gInitHUD;

#endif