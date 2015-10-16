#ifndef CKFVARS_DEFINED
#define CKFVARS_DEFINED

typedef struct
{
	qboolean bIsDead;
	int iTeam;
	int iClass;
	int iHealth;
	int iFrags;
	int iDeaths;
	int iDominates;
	int iPing;
	int iDominateList[33];
}CKFPlayerInfo;

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
}CKFPlayerStats;

class CKFClientWeapon
{
public:
	virtual BOOL CanDeploy( void ) { return TRUE; }
	virtual BOOL Deploy( void ) { return TRUE; }
	virtual BOOL ShouldWeaponIdle(void) { return FALSE; }
	virtual BOOL CanHolster(void) { return TRUE; }
	virtual void Holster( void ){}
	virtual void ItemPostFrame( void );
	virtual void PrimaryAttack( void ) {}
	virtual void SecondaryAttack( void ) {}
	virtual void Reload( void ) {}
	virtual void WeaponIdle( void ) {}
	virtual void Swing( void ) {}
	virtual int iFlags(void){ return 0;}
	virtual int iMaxClip(void){ return m_iMaxClip;}
	virtual int iMaxAmmo(void){ return m_iMaxAmmo;}
	virtual BOOL PlayEmptySound( void );
	virtual float GetMaxSpeed(void) { return 1.0; }

	virtual void SendWeaponAnim(int iAnim);
	virtual void ResetEmptySound(void);
	virtual BOOL GroupDeploy(char *szViewModel, char *szWeaponModel, int iViewAnim, int iViewBody, int iViewSkin, const char *szAnimExt);
	virtual BOOL DefaultDeploy(char *szViewModel, char *szWeaponModel, int iAnim, const char *szAnimExt);
	virtual BOOL DefaultReload( int iClipSize, int iAnim, float fDelay );

public:
	char m_szAnimExt[32];
	int m_iId;
	int m_iClip;
	int m_iMaxClip;
	int m_iAmmo;
	int m_iMaxAmmo;
	int m_fInReload;
	int m_fInSpecialReload;
	int m_iShotsFired;
	int m_iWeaponState;
	int m_bMeleeAttack;
	int m_iMeleeCrit;
	int m_bDelayedFire;
	float m_flTimeWeaponIdle;
	float m_flNextPrimaryAttack;
	float m_flNextSecondaryAttack;
	float m_flNextReload;
	float m_flDecreaseShotsFired;
	float m_flMeleeAttack;

	//no prediction
	int m_fFireOnEmpty;
	int m_iPlayEmptySound;
	int m_usFireScript;
};

class CKFClientPlayer
{
public:
	virtual BOOL PlayerCanAttack(void);
	virtual int GetCriticalFire(int iType, unsigned int iRandSeed);
	virtual void ResetMaxSpeed(void);
	virtual void Spawn(void);
	virtual void Killed(void);
	virtual int GetNumActivePipebombs(void);
	virtual int GetMedigunHealingTarget(void);
	virtual bool CanPickupBuilding(cl_entity_t *pEntity);
	virtual bool PickupBuilding(void);
public:
	int random_seed;
	entvars_t pev;
	int m_afButtonLast;
	int m_afButtonPressed;
	int m_afButtonReleased;
	float m_flNextAttack;

	int m_bCritBuff;
	int m_iCritBoost;	
	int m_iDmgDone_Recent;
	float m_fCritChance;
	
	CKFClientWeapon *m_pLastItem;
	CKFClientWeapon *m_pActiveItem;

	int m_iHealth;
	int m_iClass;
	int m_iDisguise;
	int m_iDisguiseTeam;
	int m_iDisguiseClass;
	int m_iDisguiseHealth;
	int m_iDisguiseWeapon;
	int m_iDisguiseWeaponBody;
	int m_iDisguiseSequence;
	int m_iCloak;
	float m_flCloakEnergy;

	int m_iUbercharge;
	float m_fUbercharge;
	int m_iHealer;

	int m_iBluePrintYaw;
	int m_iCarryBluePrint;
	int m_iMetal;

	int m_bAllowAttack;

	char m_szAnimExtention[32];
};

typedef struct
{
	int *g_iTeam;
	int *g_iClass;
	int *g_iDesiredClass;
	int *g_iHealth;
	int *g_iMaxHealth;
	int *g_iRoundStatus;
	int *g_iLimitTeams;
	int *g_iMaxRoundTime;
	float *g_flRoundEndTime;
	cl_entity_t **g_pTraceEntity;
	CKFPlayerInfo *g_PlayerInfo;
	CKFPlayerStats *g_PlayerStats;
	CKFClientPlayer *g_Player;
}ckf_vars_t;

enum
{
	kRenderFxCloak = 22,
	kRenderFxShadow,
	kRenderFxFireLayer,
	kRenderFxInvulnLayer
};

extern ckf_vars_t gCKFVars;

#endif

