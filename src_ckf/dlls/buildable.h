#ifndef BUILDABLE_H
#define BUILDABLE_H

#define BUILD_BUILDING (1<<0)
#define BUILD_UPGRADING (1<<1)
#define BUILD_SAPPERED (1<<2)
#define BUILD_EMPED (1<<3)
#define BUILD_ACTIVE (1<<4)

#define BUILDABLE_SENTRY 1
#define BUILDABLE_DISPENSER 2
#define BUILDABLE_ENTRANCE 3
#define BUILDABLE_EXIT 4

#define HUDBUILD_RESET 255

class CBuildSapper;

class CBaseBuildable : public CBaseMonster
{
public:
	CBaseBuildable(void)
	{
		m_pPlayer = NULL;
		m_pSapper = NULL;
		m_iLevel = 0;
		m_iFlags = 0;
		m_iUpgrade = 0;
		m_flProgress = 0;
		m_bIsRebuilt = false;
		m_iDesiredLevel = 0;
		m_iDesiredHealth = 0;
		m_iDesiredUpgrade = 0;
	}
public:
	void Restart(void){};
	int TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType, int iCrit);
	void CBaseBuildable::TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType, int iCrit);
	void Killed(entvars_t *pevAttacker, int iGib);
	int Classify(void) { return CLASS_BUILDABLE; }
	int BloodColor( void ) { return DONT_BLEED; }
	void GibMonster( void ) {}
	void Spawn(void);
public:
	virtual int GetBuildClass(void) { return -1; }
	virtual int GetBuildLevel(void) { return m_iLevel; }
	virtual int GetBuildUpgrade(void) { return m_iUpgrade; }
	virtual BOOL Repair(CBasePlayer *pPlayer);
	virtual void UpdateHUD(void){};
	virtual void UpgradeProgress(float ProgressIncrease){};
	virtual int GetSapperSequence(void){return 0;}
	virtual void Sapped(void){};
	virtual void Undeploy(void);
	virtual void Rebuild(Vector vecOrigin, Vector vecAngles);
public:
	int m_iTeam;
	CBasePlayer *m_pPlayer;
	CBuildSapper *m_pSapper;
	int m_iLevel;
	int m_iFlags;
	float m_flProgress;
	int m_iUpgrade;
protected:
	bool m_bIsRebuilt;
	int m_iDesiredLevel;
	int m_iDesiredHealth;
	int m_iDesiredUpgrade;
};

#define SENTRY_RANGE 1100
#define SENTRY_TURNRATE 160
#define SENTRY_TURNRATE_SLOW 36
#define SENTRY_MINPITCH -80
#define SENTRY_MAXPITCH 80

class CBuildSentry : public CBaseBuildable
{
public:
	void Spawn(void);
	void Precache(void);
public:
	static CBaseBuildable *CreateBuildable(Vector vecOrigin, Vector vecAngles, CBasePlayer *pOwner);
	int GetBuildClass(void) { return BUILDABLE_SENTRY; }
	void UpgradeThink(void);
	void Upgraded(void);
	void UpgradeMe(void);
	void UpgradeProgress(float ProgressIncrease);
	BOOL Repair(CBasePlayer *pPlayer);
	int GetSapperSequence(void){ return m_iLevel-1; }
public://Combat
	void Shoot(Vector &vecSrc, Vector &vecDirToEnemy);
	void ShootRocketPre(entvars_t *pevVictim, Vector vecSrc, Vector vecEnemy);
	void ShootRocket(Vector &vecSrc, Vector &vecAngles);
	void ScanThink(void);
	void RotateMe(void);
	void LookForTarget(void);
	void ActiveThink(void);
	void UpdateHUD(void);	
public:
	int m_iAmmo;
	int m_iMaxAmmo;
	int m_iRocket;
	int m_iMaxRocket;
	float m_flROF;
	float m_flNextAttack;
	float m_flNextRocket;
	int m_iPredictRocket;
//For rotate
	Vector m_vecCurAngles;
	Vector m_vecGoalAngles;
	float m_flStartYaw;
	float m_flTurnRate;
	float m_flLastSight;
	BOOL m_bStartTurn;
};

class CBuildDispenser : public CBaseBuildable
{
public:
	CBuildDispenser(void)
	{
		memset(m_Queue, 0, sizeof(m_Queue));
	}
	void Spawn(void);
	void Precache(void);
	void Killed(entvars_t *pevAttacker, int iGib);
	Vector Center(void){return pev->origin+Vector(0,0,24);};
	int FShouldCollide(CBaseEntity *pHit);
public:
	static CBaseBuildable *CreateBuildable(Vector vecOrigin, Vector vecAngles, CBasePlayer *pOwner);
	int GetBuildClass(void) { return BUILDABLE_DISPENSER; }
	void UpgradeThink(void);
	void Upgraded(void);
	void UpgradeMe(void);
	void UpgradeProgress(float ProgressIncrease);
	BOOL Repair(CBasePlayer *pPlayer);
	int GetSapperSequence(void){ return 3; }
	void Sapped(void){ClearQueue();}
public:
	void ResupplyThink(void);
	void UpdateHUD(void);
	void ScanPlayer(void);
	void JoinQueue(CBasePlayer *pPlayer);
	void ResupplyPlayer(int iAmmo, int iMetal);
	void HealPlayer(int iHeal, int iCloak);
	void CheckQueue(void);
	void ClearQueue(void);
	void DispenserBeam(void);
public:
	float m_flRadius;
	int m_iMetal;
	int m_iMaxMetal;
	float m_flNextScan;
	float m_flNextBeam;
	float m_flNextRegenerate;
	float m_flNextResupply;
	int m_Queue[33];
	bool m_bShouldUpdateHUD;
};

class CBuildTeleporter : public CBaseBuildable
{
public:
	CBuildTeleporter(void)
	{
		m_bIsEntrance = TRUE;
	}
	void Spawn(void);
	void Precache(void);
	void Killed(entvars_t *pevAttacker, int iGib);
	Vector Center(void){return pev->origin;}
	int TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType, int iCrit);
public:
	static CBaseBuildable *CreateBuildable(BOOL bIsEntrance, Vector vecOrigin, Vector vecAngles, CBasePlayer *pOwner);
	int GetBuildClass(void) { return (m_bIsEntrance) ? BUILDABLE_ENTRANCE : BUILDABLE_EXIT; }
	void BuildThink(void);
	void BuildFinish(void);
	void UpgradeMe(void);
	void UpgradeProgress(float ProgressIncrease);
	BOOL Repair(CBasePlayer *pPlayer);
	int GetSapperSequence(void){ return 4; }
public:
	void TeleportThink(void);
	void UpdateHUD(void);
	void ScanPlayer(void);
	bool CheckPlayer(CBasePlayer *pPlayer);
	void TelePlayer(void);
	void SyncTeleporter(void);
	bool ShouldRun(void);
	int GetBuildLevel(void);
	int GetBuildUpgrade(void);
	float GetChargeRate(void){return ShouldRun() ? m_flChargeRate : 0;};
public:
	CBasePlayer *m_pTelePlayer;
	CBuildTeleporter *m_pLinkTele;
	BOOL m_bIsEntrance;//管理与传送有关的一切变量
	float m_flNextCheck;
	float m_flTeleport;
	float m_flCharge;
	float m_flChargeRate;
	float m_flNextSpinSnd;
	bool m_bChargeReady;
};

void FixPlayerCrouchStuck(edict_t *pPlayer);

#endif