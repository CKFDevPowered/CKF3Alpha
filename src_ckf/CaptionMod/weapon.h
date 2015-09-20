#pragma once

#define MAX_WEAPON_SLOTS 5

#define WEAPON_NOCLIP -1

#define ROCKET_MAX_CLIP 4
#define SMG_MAX_CLIP 25
#define SHOTGUN_MAX_CLIP 6
#define STICKY_MAX_CLIP 8
#define GRENADE_MAX_CLIP 4
#define SCATTERGUN_MAX_CLIP 6
#define PISTOL_MAX_CLIP 12
#define REVOLVER_MAX_CLIP 6
#define SYRINGE_MAX_CLIP 40

#define _SNIPER_MAX_CARRY 25
#define _ROCKET_MAX_CARRY 20
#define _SMG_MAX_CARRY 75
#define _SHOTGUN_MAX_CARRY 32
#define _STICKY_MAX_CARRY 24
#define _GRENADE_MAX_CARRY 16
#define _SCATTERGUN_MAX_CARRY 32
#define _PISTOL_MAX_CARRY 36
#define _PISTOL_MAX_CARRY_ENG 200
#define _MINIGUN_MAX_CARRY 200
#define _FLAME_MAX_CARRY 200
#define _REVOLVER_MAX_CARRY 24
#define _SYRINGE_MAX_CARRY 150

#define ITEM_FLAG_SELECTONEMPTY 1
#define ITEM_FLAG_NOAUTORELOAD 2
#define ITEM_FLAG_NOAUTOSWITCHEMPTY 4
#define ITEM_FLAG_LIMITINWORLD 8
#define ITEM_FLAG_EXHAUSTIBLE 16

#define WEAPON_SLOT_PRIMARY 1
#define WEAPON_SLOT_SECONDARY 2//ckf
#define WEAPON_SLOT_MELEE 3

#define WEAPONSTATE_CHARGING (1<<3)

#include "ckfvars.h"

typedef CKFClientWeapon CClientWeapon;
typedef CKFClientPlayer CClientPlayer;

/*class CClientWeapon
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

class CClientPlayer
{
public:
	virtual BOOL PlayerCanAttack(void);
	virtual int GetCriticalFire(int iType, unsigned int iRandSeed);
	virtual void ResetMaxSpeed(void);
	virtual void Spawn(void);
	virtual void Killed(void);
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
	
	CClientWeapon *m_pLastItem;
	CClientWeapon *m_pActiveItem;

	int m_iHealth;
	int m_iClass;
	int m_iDisguise;
	int m_iDisguiseTeam;
	int m_iDisguiseClass;
	int m_iDisguiseHealth;
	int m_iDisguiseWeaponBody;
	int m_iCloak;
	float m_flCloakEnergy;

	int m_iUbercharge;
	float m_fUbercharge;

	int m_iBluePrintYaw;
	int m_iCarryBluePrint;
	int m_iMetal;

	int m_bAllowAttack;

	char m_szAnimExtention[32];
};*/

class CClientScattergun : public CClientWeapon
{
public:
	void Precache(void);
	void PrimaryAttack( void );
	BOOL Deploy(void);
	void Reload(void);
	void WeaponIdle(void);
};

class CClientPistol : public CClientWeapon
{
public:
	void Precache(void);
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy(void);
	void Reload(void);
	void WeaponIdle(void);
public:
	BOOL m_bForEngineer;
};

class CClientBat : public CClientWeapon
{
public:
	void Precache(void);
	void PrimaryAttack( void );
	BOOL Deploy(void);
	void Holster(void);
	void Swing(void);
};

class CClientRocketLauncher : public CClientWeapon
{
public:
	void Precache(void);
	void PrimaryAttack( void );
	BOOL Deploy(void);
	void Reload(void);
	void WeaponIdle(void);
public:
};

class CClientShotgun : public CClientWeapon
{
public:
	void Precache(void);
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy(void);
	void Reload(void);
	void WeaponIdle(void);
	BOOL m_bIsPrimary;
};

class CClientShovel : public CClientWeapon
{
public:
	void Precache(void);
	void PrimaryAttack( void );
	BOOL Deploy(void);
	void Holster(void);
	void Swing(void);
};

class CClientMinigun : public CClientWeapon
{
public:
	void Precache(void);
	void PrimaryAttack( void );
	BOOL CanHolster(void);
	void Holster(void);
	BOOL Deploy(void);
	void WeaponIdle(void);
	float GetMaxSpeed(void);
	void ItemPostFrame(void);
public:
	int m_iSpin;
	float m_fSpin;
};

class CClientFist : public CClientWeapon
{
public:
	void Precache(void);
	void PrimaryAttack( void );
	BOOL Deploy(void);
	void Holster(void);
	void Swing(void);
};

class CClientFlamethrower : public CClientWeapon
{
public:
	void Precache(void);
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	void Holster(void);
	BOOL Deploy(void);
	void WeaponIdle(void);
	void ItemPostFrame(void);
public:
	int m_iState;
	float m_flState;
	int m_iAmmoConsumption;
};

class CClientFireaxe : public CClientWeapon
{
public:
	void Precache(void);
	void PrimaryAttack( void );
	BOOL Deploy(void);
	void Holster(void);
	void Swing(void);
public:
};

class CClientGrenadeLauncher : public CClientWeapon
{
public:
	void Precache(void);
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy(void);
	void Reload(void);
	void WeaponIdle(void);
public:
};

class CClientStickyLauncher : public CClientWeapon
{
public:
	void Precache(void);
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy(void);
	void Holster(void);
	void Reload(void);
	void WeaponIdle(void);
	void StopCharge(void);
	void ItemPostFrame(void);
	void StickyLauncherFire(void);
public:
	float m_fCharge;
	float m_fChargeTimer;
	int m_iStickyNum;
	int m_iMaxDeploy;
};

class CClientBottle : public CClientWeapon
{
public:
	void Precache(void);
	void PrimaryAttack( void );
	BOOL Deploy(void);
	void Holster(void);
	void Swing(void);
public:
	int m_bBroken;
};

class CClientSniperifle : public CClientWeapon
{
public:
	void Precache(void);
	void PrimaryAttack(void);
	void SecondaryAttack(void);
	BOOL Deploy(void);
	void WeaponIdle(void);
	BOOL CanHolster(void);
	void Holster(void);
	void ItemPostFrame(void);
	void StartZoom(void);
	void StopZoom(void);
	float GetMaxSpeed(void);
public:
	int m_iResumeZoom;
	int m_fSpotActive;
	float m_fCharge;
	float m_fChargeTimer;
};

class CClientSMG : public CClientWeapon
{
public:
	void Precache(void);
	void PrimaryAttack( void );
	BOOL Deploy(void);
	void Reload(void);
	void WeaponIdle(void);
};

class CClientKukri : public CClientWeapon
{
public:
	void Precache(void);
	void PrimaryAttack( void );
	BOOL Deploy(void);
	void Holster(void);
	void Swing(void);
public:
};

class CClientSyringegun : public CClientWeapon
{
public:
	void Precache(void);
	void PrimaryAttack( void );
	BOOL Deploy(void);
	void Reload(void);
	void WeaponIdle(void);
public:
};

class CClientMedigun : public CClientWeapon
{
public:
	void Precache(void);
	void PrimaryAttack( void );
	BOOL Deploy(void);
	void WeaponIdle(void);
public:
	int m_iHealTarget;
};

class CClientBonesaw : public CClientWeapon
{
public:
	void Precache(void);
	void PrimaryAttack( void );
	BOOL Deploy(void);
	void Holster(void);
	void Swing(void);
public:
};

class CClientWrench : public CClientWeapon
{
public:
	void Precache(void);
	void PrimaryAttack( void );
	void SecondaryAttack( void );
	BOOL Deploy(void);
	void Holster(void);
	void Swing(void);
public:
};

class CClientBuildPDA : public CClientWeapon
{
public:
	void Precache(void);
	void SecondaryAttack( void );
	BOOL Deploy(void);
	void Holster(void);
	BOOL CanHolster(void);
	void WeaponIdle(void);
public:
};

class CClientDestroyPDA : public CClientWeapon
{
public:
	void Precache(void);
	BOOL Deploy(void);
	void Holster(void);
	BOOL CanHolster(void);
	void WeaponIdle(void);
public:
};

class CClientRevolver : public CClientWeapon
{
public:
	void Precache(void);
	void PrimaryAttack( void );
	BOOL Deploy(void);
	void Reload(void);
	void WeaponIdle(void);
};

class CClientButterfly : public CClientWeapon
{
public:
	void Precache(void);
	void PrimaryAttack( void );
	BOOL Deploy(void);
	void Holster(void);
	void Swing(void);
	void ItemPostFrame(void);
public:

};

class CClientSapper : public CClientWeapon
{
public:
	void Precache(void);
	BOOL Deploy(void);
	void ItemPostFrame(void);
public:
};

class CClientDisguiseKit : public CClientWeapon
{
public:
	void Precache(void);
	BOOL Deploy(void);
	void Holster(void);
	void WeaponIdle(void);
public:
};

extern CClientPlayer g_Player;
extern CClientWeapon *g_pClientWeapon[MAX_WEAPON_SLOTS];
extern CClientWeapon *g_pWeapons[MAX_WEAPONS];

float UTIL_WeaponTimeBase(void);
void HUD_PlaybackEvent(int flags, const edict_t *pInvoker, unsigned short eventindex, float delay, float *origin, float *angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2 );
#define PLAYBACK_EVENT_FULL HUD_PlaybackEvent
#define PRECACHE_EVENT gEngfuncs.pfnPrecacheEvent

extern local_state_t *g_finalstate;
extern int g_runfuncs;

#define REGISTER_WEAPON(name, id) {g_pWeapons[id] = (CClientWeapon *)(&g_##name);g_##name.Precache();}