#ifndef WEAPONS_H
#define WEAPONS_H

#include "effects.h"
#include "buildable.h"

class CBasePlayer;
extern int gmsgWeapPickup;
extern int gmsgReloadSound;

#define PJ_AIRBLAST_DEFLECTABLE (1<<0)
#define PJ_AIRBLAST_OWNER (1<<1)
#define PJ_AIRBLAST_UPWARD (1<<2)
#define PJ_AIRBLAST_ROTATE (1<<3)
#define PJ_AIRBLAST_DEFLECTED (1<<4)
#define PJ_AIRBLAST_TEMPOWNER (1<<5)

extern void FindHullIntersection(const Vector &vecSrc, TraceResult &tr, float *pflMins, float *pfkMaxs, edict_t *pEntity);

class CGrenade : public CBaseMonster
{
public:
	//void Spawn(void);
	//int Save(CSave &save);
	//int Restore(CRestore &restore);
	//int ObjectCaps(void) { return m_bIsC4 != FALSE ? FCAP_CONTINUOUS_USE : 0; }
	virtual int BloodColor(void) { return DONT_BLEED; }
	//void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	virtual int TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType);
	virtual int Classify(void) { return CLASS_PROJECTILE; }
	virtual int FShouldCollide(CBaseEntity *pHit);
	virtual void Killed(entvars_t *pevAttacker, int iGib){}
public:
	//ckf
	void CKFSmoke(void);
	void CKFDisappear(void);
	void CKFExplode(TraceResult *pTrace, int bitsDamageType);
	virtual void Deflected(CBaseEntity *pAttacker, Vector vecDirShooting, float flForce){};
	virtual void CKFDetonate(void){};
	virtual bool IsDeflected(void){ return (m_iPjFlags & PJ_AIRBLAST_DEFLECTED) ? true : false;}

public:
	//static TYPEDESCRIPTION m_SaveData[];

public:
	int m_iTeam;
	int m_iCrit;
	float m_fDmg;
	float m_fDmgRadius;
	float m_fForce;
	float m_fForceRadius;
	float m_fGCharge;
	int m_iPjFlags;
	CBasePlayer *m_pDeflecter;
public:
	TraceResult m_iTrace;
};

#define ITEM_HEALTHKIT 1
#define ITEM_ANTIDOTE 2
#define ITEM_SECURITY 3
#define ITEM_BATTERY 4

#define WEAPON_NONE 0

#define WEAPON_SLOT_PRIMARY 1
#define WEAPON_SLOT_SECONDARY 2//ckf
#define WEAPON_SLOT_MELEE 3

#define WEAPON_SLOT_PISTOL 2

#define WEAPON_SLOT_PDA 4
#define WEAPON_SLOT_PDA2 5
#define WEAPON_SLOT_KNIFE 3
#define WEAPON_SLOT_GRENADE 4
#define WEAPON_SLOT_C4 5

//ckf
#define WEAPON_ROCKETLAUNCHER 1
#define WEAPON_SNIPERIFLE 2
#define WEAPON_SMG 3
#define WEAPON_KUKRI 4
#define WEAPON_SHOTGUN 5
#define WEAPON_SHOVEL 6
#define WEAPON_STICKYLAUNCHER 7
#define WEAPON_GRENADELAUNCHER 8
#define WEAPON_BOTTLE 9
#define WEAPON_SCATTERGUN 10
#define WEAPON_PISTOL 11
#define WEAPON_BAT 12
#define WEAPON_MINIGUN 13
#define WEAPON_FIST 14
#define WEAPON_FIREAXE 15
#define WEAPON_FLAMETHROWER 16
#define WEAPON_REVOLVER 17
#define WEAPON_SAPPER 18
#define WEAPON_BUTTERFLY 19
#define WEAPON_SYRINGEGUN 20
#define WEAPON_MEDIGUN 21
#define WEAPON_BONESAW 22
#define WEAPON_DISGUISEKIT 23
#define WEAPON_WRENCH 24
#define WEAPON_BUILDPDA 25
#define WEAPON_DESTROYPDA 26

#define WEAPON_ALLWEAPONS (~(1 << WEAPON_SUIT))
#define WEAPON_SUIT 31
#define MAX_WEAPONS 32

//ckf
#define ROCKETLAUNCHER_WEIGHT 30
#define SNIPERIFLE_WEIGHT 30
#define SMG_WEIGHT 20
#define KUKRI_WEIGHT 10
#define SHOTGUN_WEIGHT 20
#define SHOVEL_WEIGHT 10
#define STICKYLAUNCHER_WEIGHT 20
#define GRENADELAUNCHER_WEIGHT 30
#define BOTTLE_WEIGHT 10
#define SCATTERGUN_WEIGHT 30
#define PISTOL_WEIGHT 20
#define BAT_WEIGHT 10
#define MINIGUN_WEIGHT 30
#define FIST_WEIGHT 10
#define FLAMETHROWER_WEIGHT 30
#define FIREAXE_WEIGHT 10
#define REVOLVER_WEIGHT 30
#define SAPPER_WEIGHT 20
#define BUTTERFLY_WEIGHT 10
#define SYRINGEGUN_WEIGHT 30
#define MEDIGUN_WEIGHT 20
#define BONESAW_WEIGHT 10
#define DISGUISEKIT_WEIGHT 10
#define WRENCH_WEIGHT 10
#define BUILDPDA_WEIGHT 2
#define DESTROYPDA_WEIGHT 1
#define DESTROYPDA_WEIGHT 1

#define MAX_NORMAL_BATTERY 100

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

#define ROCKET_DEFAULT_GIVE 24
#define SNIPERIFLE_DEFAULT_GIVE 25
#define SMG_DEFAULT_GIVE 100
#define SHOTGUN_DEFAULT_GIVE 38
#define STICKY_DEFAULT_GIVE 32
#define GRENADE_DEFAULT_GIVE 20
#define SCATTERGUN_DEFAULT_GIVE 38
#define PISTOL_DEFAULT_GIVE 48
#define MINIGUN_DEFAULT_GIVE 200
#define FLAME_DEFAULT_GIVE 200
#define REVOLVER_DEFAULT_GIVE 30
#define SYRINGEGUN_DEFAULT_GIVE 190

#define AMMO_9MM_GIVE 30
#define AMMO_BUCKSHOT_GIVE 8
#define AMMO_556NATO_GIVE 30
#define AMMO_556NATOBOX_GIVE 30
#define AMMO_762NATO_GIVE 30
#define AMMO_45ACP_GIVE 12
#define AMMO_50AE_GIVE 7
#define AMMO_338MAGNUM_GIVE 10
#define AMMO_57MM_GIVE 50
#define AMMO_357SIG_GIVE 13

#define AMMO_ROCKET_GIVE 24
#define AMMO_SNIPER_GIVE 25
#define AMMO_SMG_GIVE 100
#define AMMO_SHOTGUN_GIVE 38
#define AMMO_STICKY_GIVE 32
#define AMMO_GRENADE_GIVE 20
#define AMMO_SCATTERGUN_GIVE 38
#define AMMO_MINIGUN_GIVE 200
#define AMMO_FLAME_GIVE 200
#define AMMO_REVOLVER_GIVE 30
#define AMMO_SYRINGEGUN_GIVE 190

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
//ckf

#define WEAPONSTATE_CHARGING (1<<3)

typedef enum
{
	BULLET_NONE = 0,
	BULLET_PLAYER_9MM,
	BULLET_PLAYER_MP5,
	BULLET_PLAYER_357,
	BULLET_PLAYER_BUCKSHOT,
	BULLET_PLAYER_CROWBAR,

	BULLET_MONSTER_9MM,
	BULLET_MONSTER_MP5,
	BULLET_MONSTER_12MM,

	BULLET_PLAYER_SNIPER,
	BULLET_PLAYER_SNIPER_NOHS,
	BULLET_PLAYER_TF2,
	BULLET_SENTRY_TF2
}
Bullet;

#define DIST_MEDIUM 409
#define DIST_LONG 819

#define TRACE_NEVER -1
#define TRACE_ATTACHMENT_1 0
#define TRACE_ATTACHMENT_2 1
#define TRACE_ATTACHMENT_3 2
#define TRACE_ATTACHMENT_4 3

#define ITEM_FLAG_SELECTONEMPTY 1
#define ITEM_FLAG_NOAUTORELOAD 2
#define ITEM_FLAG_NOAUTOSWITCHEMPTY 4
#define ITEM_FLAG_LIMITINWORLD 8
#define ITEM_FLAG_EXHAUSTIBLE 16
#define WEAPON_IS_ONTARGET 0x40

typedef struct
{
	int iSlot;
	int iPosition;
	const char *pszAmmo1;
	int iMaxAmmo1;
	const char *pszAmmo2;
	int iMaxAmmo2;
	const char *pszName;
	int iMaxClip;
	int iId;
	int iFlags;
	int iWeight;
}
ItemInfo;

typedef struct
{
	const char *pszName;
	int iId;
}
AmmoInfo;

class CBasePlayerItem : public CBaseAnimating
{
public:
	virtual void SetObjectCollisionBox(void);
	virtual int AddToPlayer(CBasePlayer *pPlayer);
	virtual int AddDuplicate(CBasePlayerItem *pItem) { return FALSE; }
	virtual int GetItemInfo(ItemInfo *p) { return 0; }
	virtual BOOL CanDeploy(void) { return TRUE; }
	virtual BOOL CanDrop(void) { return FALSE; }
	virtual BOOL Deploy(void) { return TRUE; }
	virtual BOOL IsWeapon(void) { return FALSE; }
	virtual BOOL CanHolster(void) { return TRUE; }
	virtual void Holster(int skiplocal = 0);
	virtual void UpdateItemInfo(void) {}
	virtual void ItemPreFrame(void) {}
	virtual void ItemPostFrame(void) {}
	virtual void Drop(void);
	virtual void Kill(void);
	virtual void AttachToPlayer(CBasePlayer *pPlayer);
	virtual int PrimaryAmmoIndex(void) { return -1; }
	virtual int SecondaryAmmoIndex(void) { return -1; }
	virtual int UpdateClientData(CBasePlayer *pPlayer) { return 0; }
	virtual CBasePlayerItem *GetWeaponPtr(void) { return NULL; }
	virtual float GetMaxSpeed(void) { return 1; }
	virtual int iItemSlot(void) { return 0; }
	virtual void PreAttachPlayer(CBasePlayer *pPlayer){};

public:
	void EXPORT DestroyItem(void);
	void EXPORT DefaultTouch(CBaseEntity *pOther);
	void EXPORT FallThink(void);
	void EXPORT Materialize(void);
	void EXPORT AttemptToMaterialize(void);
	CBaseEntity *Respawn(void);
	void FallInit(void);
	void CheckRespawn(void);

public:
	static ItemInfo ItemInfoArray[MAX_WEAPONS];
	static AmmoInfo AmmoInfoArray[MAX_AMMO_SLOTS];

public:
	CBasePlayer *m_pPlayer;
	CBasePlayerItem *m_pNext;
	int m_iId;
public:
	virtual int iBody(void) { return 0; }
	virtual int iSkin(void) { return 0; }
	virtual int iItemPosition(void) { return ItemInfoArray[m_iId].iPosition; }
	virtual const char *pszAmmo1(void) { return ItemInfoArray[m_iId].pszAmmo1; }
	virtual int iMaxAmmo1(void) { return ItemInfoArray[m_iId].iMaxAmmo1; }
	virtual const char *pszAmmo2(void) { return ItemInfoArray[m_iId].pszAmmo2; }
	virtual int iMaxAmmo2(void) { return ItemInfoArray[m_iId].iMaxAmmo2; }
	virtual const char *pszName(void) { return ItemInfoArray[m_iId].pszName; }
	virtual int iMaxClip(void) { return ItemInfoArray[m_iId].iMaxClip; }
	virtual int iWeight(void) { return ItemInfoArray[m_iId].iWeight; }
	virtual int iFlags(void) { return ItemInfoArray[m_iId].iFlags; }
};

class CBasePlayerWeapon : public CBasePlayerItem
{
public:
	virtual int AddToPlayer(CBasePlayer *pPlayer);
	virtual int AddDuplicate(CBasePlayerItem *pItem);
	virtual int ExtractAmmo(CBasePlayerWeapon *pWeapon);
	virtual int ExtractClipAmmo(CBasePlayerWeapon *pWeapon);
	virtual int AddWeapon(void) { ExtractAmmo(this); return TRUE; }
	virtual void UpdateItemInfo(void) {};
	virtual BOOL PlayEmptySound(void);
	virtual void ResetEmptySound(void);
	virtual void SendWeaponAnim(int iAnim, int skiplocal = 0);
	virtual void SendWeaponAnimEx(int iAnim, int iBody, int iSkin, int skiplocal = 0);
	virtual BOOL CanDeploy(void);
	virtual BOOL IsWeapon(void) { return TRUE; }
	virtual BOOL IsUseable(void);
	virtual void ItemPostFrame(void);
	virtual void PrimaryAttack(void) {}
	virtual void SecondaryAttack(void) {}
	virtual void Reload(void) {}
	virtual void WeaponIdle(void) {}
	virtual int UpdateClientData(CBasePlayer *pPlayer);
	virtual void RetireWeapon(void);
	virtual BOOL ShouldWeaponIdle(void) { return FALSE; }
	virtual void Holster(int skiplocal = 0);
	virtual BOOL UseDecrement(void) { return FALSE; }
	virtual CBasePlayerItem *GetWeaponPtr(void) { return (CBasePlayerItem *)this; }
	virtual void Swing(void){};//for melee attack
	virtual void Reset(void){};//for reset weapon statement
	virtual void Reloaded(void);

public:
	BOOL DefaultDeploy(char *szViewModel, char *szWeaponModel, int iAnim, char *szAnimExt, int body = 0);
	BOOL GroupDeploy(char *szViewModel, char *szWeaponModel, int iViewAnim, int iViewBody, int iViewSkin, char *szAnimExt, int iWpnBody);
	int DefaultReload(int iClipSize, int iAnim, float fDelay, int body = 0);
	void ReloadSound(void);
	BOOL AddPrimaryAmmo(int iCount, char *szName, int iMaxClip, int iMaxCarry);
	BOOL AddSecondaryAmmo(int iCount, char *szName, int iMaxCarry);
	int PrimaryAmmoIndex(void);
	int SecondaryAmmoIndex(void);
	//bool HasAnyAmmo( void );

public:
	int m_iPlayEmptySound;
	int m_fFireOnEmpty;
	float m_flNextPrimaryAttack;
	float m_flNextSecondaryAttack;
	float m_flTimeWeaponIdle;
	int m_iPrimaryAmmoType;
	int m_iSecondaryAmmoType;
	int m_iClip;
	int m_iClientClip;
	int m_iClientWeaponState;
	int m_fInReload;
	int m_fInSpecialReload;
	int m_iDefaultAmmo;
	int m_iShellId;
	int m_iShotsFired;
	int m_iWeaponState;
	float m_flNextReload;
	float m_flDecreaseShotsFired;
	float m_flMeleeAttack;
	int m_iMeleeCrit;
	BOOL m_bMeleeAttack;
	BOOL m_bDelayedFire;//必须松开+attack再按下才有效果
	int m_usFireScript;//event script
};

class CBasePlayerAmmo : public CBaseEntity
{
public:
	virtual void Spawn(void);
};

extern DLL_GLOBAL short g_sModelIndexLaser;
extern DLL_GLOBAL const char *g_pModelNameLaser;
extern DLL_GLOBAL short g_sModelIndexLaserDot;
extern DLL_GLOBAL short g_sModelIndexFireball;
extern DLL_GLOBAL short g_sModelIndexFireball2;
extern DLL_GLOBAL short g_sModelIndexFireball3;
extern DLL_GLOBAL short g_sModelIndexFireball4;
extern DLL_GLOBAL short g_sModelIndexSmoke;
extern DLL_GLOBAL short g_sModelIndexSmokePuff;
extern DLL_GLOBAL short g_sModelIndexBubbles;
extern DLL_GLOBAL short g_sModelIndexBloodDrop;
extern DLL_GLOBAL short g_sModelIndexBloodSpray;
extern DLL_GLOBAL short g_sModelIndexRadio;

extern void ClearMultiDamage(void);
extern void ApplyMultiDamage(entvars_t *pevInflictor, entvars_t *pevAttacker);
extern void AddMultiDamage(entvars_t *pevInflictor, CBaseEntity *pEntity, float flDamage, int bitsDamageType, int iCrit);
extern void DecalGunshot(TraceResult *pTrace, int iBulletType, BOOL bShowUnEffect, entvars_t *pevAttacker, BOOL bShowSparks);
extern void SpawnBlood(Vector vecSpot, int bloodColor, float flDamage);
extern void RadiusFlash(Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage);
extern void CKFRadiusDamage(Vector vecSrc, entvars_t *pevDirectHit, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType, float flForce, float flForceRadius, int iCrit);
extern void CKFAirblast(Vector vecSrc, Vector vecDirShooting, entvars_t *pevInflictor, entvars_t *pevAttacker, int iClassIgnore, float flForce, float flForceRadius);

typedef struct 
{
	CBaseEntity *pEntity;
	float amount;
	int type;
	int crit;
}
MULTIDAMAGE;

extern MULTIDAMAGE gMultiDamage;

//ckf
#define UC_INVULNERABLE (1<<0)
#define UC_KRITKRIEG (1<<1)

#define LOUD_GUN_VOLUME 1000
#define NORMAL_GUN_VOLUME 600
#define QUIET_GUN_VOLUME 200

#define BRIGHT_GUN_FLASH 512
#define NORMAL_GUN_FLASH 256
#define DIM_GUN_FLASH 128

#define BIG_EXPLOSION_VOLUME 2048
#define NORMAL_EXPLOSION_VOLUME 1024
#define SMALL_EXPLOSION_VOLUME 512

#define WEAPON_ACTIVITY_VOLUME 64

#define VECTOR_CONE_1DEGREES Vector(0.00873, 0.00873, 0.00873)
#define VECTOR_CONE_2DEGREES Vector(0.01745, 0.01745, 0.01745)
#define VECTOR_CONE_3DEGREES Vector(0.02618, 0.02618, 0.02618)
#define VECTOR_CONE_4DEGREES Vector(0.03490, 0.03490, 0.03490)
#define VECTOR_CONE_5DEGREES Vector(0.04362, 0.04362, 0.04362)
#define VECTOR_CONE_6DEGREES Vector(0.05234, 0.05234, 0.05234)
#define VECTOR_CONE_7DEGREES Vector(0.06105, 0.06105, 0.06105)
#define VECTOR_CONE_8DEGREES Vector(0.06976, 0.06976, 0.06976)
#define VECTOR_CONE_9DEGREES Vector(0.07846, 0.07846, 0.07846)
#define VECTOR_CONE_10DEGREES Vector(0.08716, 0.08716, 0.08716)
#define VECTOR_CONE_15DEGREES Vector(0.13053, 0.13053, 0.13053)
#define VECTOR_CONE_20DEGREES Vector(0.17365, 0.17365, 0.17365)

class CWeaponBox : public CBaseEntity
{
public:
	void Spawn(void);
	void Precache(void);
	void KeyValue(KeyValueData *pkvd);
	void Touch(CBaseEntity *pOther);
	void SetObjectCollisionBox(void);
	int Classify(void) { return CLASS_WEAPONBOX; }
	void TouchGround(CBaseEntity *pOther);
public:
	float m_flGivePercent;
};

//#ifdef PLAYER_H//VSSB

class CRocketLauncher : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int GetItemInfo(ItemInfo *p);
	BOOL CanDrop(void) { return FALSE; }
	BOOL Deploy(void);
	float GetMaxSpeed(void) { return 1; }
	int iItemSlot(void) { return WEAPON_SLOT_PRIMARY; }
	void PrimaryAttack(void);
	void Reload(void);
	void WeaponIdle(void);
	void Reloaded(void);

	BOOL UseDecrement(void)
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}	

private:
	void RocketLauncherFire();
};

class CRocket : public CGrenade
{
public:
	void Spawn(void);
	void Precache(void);
	void RocketTouch(CBaseEntity *pOther);
	void RocketLaunch(void);
	static CRocket *CreatePjRocket( Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner );
	void Deflected(CBaseEntity *pAttacker, Vector vecDirShooting, float flForce);
	void Killed(entvars_t *pevAttacker, int iGib);
};

class CLaserSpot : public CBaseEntity
{
	void Spawn( void );
	void Precache( void );

	int	ObjectCaps( void ) { return FCAP_DONT_SAVE; }
	
public:
	static CLaserSpot *CreateSpot( CBaseEntity *pOwner );
};

class CSniperifle : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int GetItemInfo(ItemInfo *p);
	BOOL CanDrop(void) { return FALSE; }
	BOOL Deploy(void);
	BOOL CanHolster(void);
	void Holster(int skiplocal);
	float GetMaxSpeed(void);
	int iItemSlot(void) { return WEAPON_SLOT_PRIMARY; }
	void PrimaryAttack(void);
	void SecondaryAttack(void);
	void WeaponIdle(void);
	void UpdateSpot(void);
	void ItemPostFrame(void);
	void StartZoom(void);
	void StopZoom(void);

	BOOL UseDecrement(void)
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	void SniperifleFire(void);

public:
	int m_iResumeZoom;
	int m_fSpotActive;
	float m_fCharge;
	float m_fChargeTimer;
	CLaserSpot *m_pSpot;
	CLaserSpot *m_pDSpot;//Dark Spot behind the bright spot
};

class CSMG : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int GetItemInfo(ItemInfo *p);
	BOOL CanDrop(void) { return FALSE; }
	BOOL Deploy(void);
	float GetMaxSpeed(void) { return 1; }
	int iItemSlot(void) { return WEAPON_SLOT_SECONDARY; }
	void PrimaryAttack(void);
	void Reload(void);
	void WeaponIdle(void);

	BOOL UseDecrement(void)
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	void SMGFire();
};

class CKukri : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int GetItemInfo(ItemInfo *p);
	BOOL CanDrop(void) { return FALSE; }
	BOOL Deploy(void);
	void Holster(int skiplocal);
	float GetMaxSpeed(void) { return 1; }
	int iItemSlot(void) { return WEAPON_SLOT_KNIFE; }
	void PrimaryAttack(void);

	BOOL UseDecrement(void)
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

public:
	void Swing(void);
};

class CShotgun : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int GetItemInfo(ItemInfo *p);
	BOOL Deploy(void);
	float GetMaxSpeed(void) { return 1; }
	int iItemSlot(void) { return (!m_bIsPrimary) ? WEAPON_SLOT_SECONDARY : WEAPON_SLOT_PRIMARY; }
	void PrimaryAttack(void);
	void SecondaryAttack(void);
	void Reload(void);
	void WeaponIdle(void);
	void PreAttachPlayer(CBasePlayer *pPlayer);
	void Reloaded(void);

	int iSlot(void);
	const char *pszAmmo1(void);
	int iWeight(void);
	int iMaxAmmo1(void);
	int iBody(void);

	BOOL UseDecrement(void)
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	float m_flPumpTime;
	BOOL m_bIsPrimary;
};

class CShovel : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int GetItemInfo(ItemInfo *p);
	BOOL CanDrop(void) { return FALSE; }
	BOOL Deploy(void);
	void Holster(int skiplocal);
	float GetMaxSpeed(void) { return 1; }
	int iItemSlot(void) { return WEAPON_SLOT_KNIFE; }
	void PrimaryAttack(void);

	BOOL UseDecrement(void)
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

public:
	void Swing(void);

private:
	TraceResult m_trHit;
};

class CStickyLauncher;
class CSticky;

class CSticky : public CGrenade
{
public:
	void Spawn(void);
	void Precache(void);
	void Killed(entvars_t *pevAttacker, int iGib);
	void StickyThink(void);
	void StickyTouch( CBaseEntity *pOther );
	static CSticky *CreatePjSticky( Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner );
	void Deflected(CBaseEntity *pAttacker, Vector vecDirShooting, float flForce);
	void CKFDeflectReset(void);
	void CKFDetonate(void);
public:
	BOOL m_bDetonating;
	float m_fDeploy;
};

class CStickyLauncher : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int GetItemInfo(ItemInfo *p);
	BOOL CanDrop(void) { return FALSE; }
	BOOL Deploy(void);
	void Holster(int skiplocal);
	float GetMaxSpeed(void) { return 1; }
	int iItemSlot(void) { return WEAPON_SLOT_SECONDARY; }
	void PrimaryAttack(void);
	void SecondaryAttack(void);
	void ItemPostFrame(void);	
	void StickyFucker(void);
	int RecountSticky(void);
	void Reload(void);
	void Reloaded(void);
	void WeaponIdle(void);
	void StopCharge(void);

	BOOL UseDecrement(void)
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

	int m_iStickyNum;

private:
	void StickyLauncherFire();

public:
	int m_iMaxDeploy;
	float m_fCharge;
	float m_fChargeTimer;
};

class CDemoGrenade : public CGrenade
{
public:
	void Spawn(void);
	void Precache(void);
	void GrenadeTouch( CBaseEntity *pOther );
	void GrenadeThink(void);
	static CDemoGrenade *CreateDemoGrenade( Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner );
	void Deflected(CBaseEntity *pAttacker, Vector vecDirShooting, float flForce);
	void CKFDetonate(void);
	void Killed(entvars_t *pevAttacker, int iGib);
public:
	BOOL m_bFall;
	BOOL m_bRoll;
	int m_iJumpTimes;
};

class CGrenadeLauncher : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int GetItemInfo(ItemInfo *p);
	BOOL CanDrop(void) { return FALSE; }
	BOOL Deploy(void);
	float GetMaxSpeed(void) { return 1; }
	int iItemSlot(void) { return WEAPON_SLOT_PRIMARY; }
	void PrimaryAttack(void);
	void SecondaryAttack(void);
	void Reload(void);
	void WeaponIdle(void);
	void Reloaded(void);

	BOOL UseDecrement(void)
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	void GrenadeLauncherFire();
};

class CBottle : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int GetItemInfo(ItemInfo *p);
	BOOL CanDrop(void) { return FALSE; }
	BOOL Deploy(void);
	void Holster(int skiplocal);
	float GetMaxSpeed(void) { return 1; }
	int iItemSlot(void) { return WEAPON_SLOT_KNIFE; }
	void PrimaryAttack(void);
	void SecondaryAttack(void);
	void Reset(void);

	BOOL UseDecrement(void)
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

public:
	void Swing(void);

public:
	BOOL m_bBroken;
};

class CScattergun : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int GetItemInfo(ItemInfo *p);
	BOOL CanDrop(void) { return FALSE; }
	BOOL Deploy(void);
	float GetMaxSpeed(void) { return 1; }
	int iItemSlot(void) { return WEAPON_SLOT_PRIMARY; }
	void PrimaryAttack(void);
	void Reload(void);
	void WeaponIdle(void);
	void Reloaded(void);

	BOOL UseDecrement(void)
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	float m_flPumpTime;
};

class CPistol : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int GetItemInfo(ItemInfo *p);
	BOOL CanDrop(void) { return FALSE; }
	BOOL Deploy(void);
	float GetMaxSpeed(void) { return 1; }
	int iItemSlot(void) { return WEAPON_SLOT_SECONDARY; }
	void PrimaryAttack(void);
	void SecondaryAttack(void);
	void Reload(void);
	void WeaponIdle(void);
	void PreAttachPlayer(CBasePlayer *pPlayer);

	BOOL UseDecrement(void)
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	void PistolFire();
	int iMaxAmmo1(void);
	BOOL m_bForEngineer;
};

class CBat : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int GetItemInfo(ItemInfo *p);
	BOOL CanDrop(void) { return FALSE; }
	BOOL Deploy(void);
	void Holster(int skiplocal);
	float GetMaxSpeed(void) { return 1; }
	int iItemSlot(void) { return WEAPON_SLOT_KNIFE; }
	void PrimaryAttack(void);

	BOOL UseDecrement(void)
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

public:
	void Swing(void);
};

class CMinigun : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int GetItemInfo(ItemInfo *p);
	BOOL CanDrop(void) { return FALSE; }
	BOOL Deploy(void);
	void Holster(int skiplocal);
	float GetMaxSpeed(void);
	int iItemSlot(void) { return WEAPON_SLOT_PRIMARY; }
	void PrimaryAttack(void);
	void WeaponIdle(void);
	void ItemPostFrame(void);
	BOOL CanHolster(void);

	BOOL UseDecrement(void)
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

public:
	int m_iSpin;
	float m_fSpin;
};

class CFist : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int GetItemInfo(ItemInfo *p);
	BOOL CanDrop(void) { return FALSE; }
	BOOL Deploy(void);
	void Holster(int skiplocal);
	float GetMaxSpeed(void) { return 1; }
	int iItemSlot(void) { return WEAPON_SLOT_KNIFE; }
	void PrimaryAttack(void);

	BOOL UseDecrement(void)
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

public:
	void Swing(void);
};

class CFlamethrower : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int GetItemInfo(ItemInfo *p);
	BOOL CanDrop(void) { return FALSE; }
	BOOL Deploy(void);
	void Holster(int skiplocal);
	float GetMaxSpeed(void) { return 1; }
	int iItemSlot(void) { return WEAPON_SLOT_PRIMARY; }
	void PrimaryAttack(void);
	void SecondaryAttack(void);
	void WeaponIdle(void);

	BOOL UseDecrement(void)
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

public:
	void FlamethrowerFire(void);
	void FlamethrowerAirblast(void);
	int m_iAmmoConsumption;
	float m_flState;
	int m_iState;
};

class CFlame : public CGrenade
{
public:
	void Spawn(void);
	void Precache(void);
	void EXPORT FlameTouch(CBaseEntity *pOther);
	void EXPORT FlameThink(void);
	static CFlame *CreateFlame( Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner );
private:
	float m_fFireTime;
	float m_fDmgTime;
};

class CFireaxe : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int GetItemInfo(ItemInfo *p);
	BOOL CanDrop(void) { return FALSE; }
	BOOL Deploy(void);
	void Holster(int skiplocal);
	float GetMaxSpeed(void) { return 1; }
	int iItemSlot(void) { return WEAPON_SLOT_KNIFE; }
	void PrimaryAttack(void);

	BOOL UseDecrement(void)
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

public:
	void Swing(void);
};

class CRevolver : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int GetItemInfo(ItemInfo *p);
	BOOL CanDrop(void) { return FALSE; }
	BOOL Deploy(void);
	float GetMaxSpeed(void) { return 1; }
	int iItemSlot(void) { return WEAPON_SLOT_PRIMARY; }
	void PrimaryAttack(void);
	void SecondaryAttack(void);
	void Reload(void);
	void WeaponIdle(void);

	BOOL UseDecrement(void)
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	void RevolverFire();
};

class CSapper : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int GetItemInfo(ItemInfo *p);
	BOOL CanDrop(void) { return FALSE; }
	BOOL Deploy(void);
	float GetMaxSpeed(void) { return 1; }
	int iItemSlot(void) { return WEAPON_SLOT_SECONDARY; }
	void PrimaryAttack(void);
	void SecondaryAttack(void);
	void WeaponIdle(void);

	BOOL UseDecrement(void)
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
};

class CBuildSapper : public CGrenade
{
public:
	void Spawn(void);
	void Precache(void);
	void EXPORT SapperThink(void);
	static CBuildSapper *CreateBuildSapper(CBaseBuildable *pBuild, CBaseEntity *pOwner );
	void Killed(entvars_t *pevAttacker, int iGib);
	int TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType);
public:
	CBaseBuildable *m_pBuild;
};

class CButterfly : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int GetItemInfo(ItemInfo *p);
	BOOL CanDrop(void) { return FALSE; }
	BOOL Deploy(void);
	void Holster(int skiplocal);
	float GetMaxSpeed(void) { return 1; }
	int iItemSlot(void) { return WEAPON_SLOT_KNIFE; }
	void PrimaryAttack(void);
	void SecondaryAttack(void);

	BOOL UseDecrement(void)
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

public:
	void Swing(void);
};

class CSyringeGun : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int GetItemInfo(ItemInfo *p);
	BOOL CanDrop(void) { return FALSE; }
	BOOL Deploy(void);
	float GetMaxSpeed(void) { return 1; }
	int iItemSlot(void) { return WEAPON_SLOT_PRIMARY; }
	void PrimaryAttack(void);
	void Reload(void);
	void WeaponIdle(void);

	BOOL UseDecrement(void)
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

private:
	void SyringeGunFire();
};

class CSyringe : public CGrenade
{
public:
	void Spawn(void);
	void Precache(void);
	void SyringeTouch( CBaseEntity *pOther );
	void SyringeThink(void);
	static CSyringe *CreatePjSyringe( Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner );
};

class CMedigun : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int GetItemInfo(ItemInfo *p);
	BOOL CanDrop(void) { return FALSE; }
	BOOL Deploy(void);
	void Holster(int skiplocal);
	float GetMaxSpeed(void) { return 1; }
	int iItemSlot(void) { return WEAPON_SLOT_SECONDARY; }
	void PrimaryAttack(void);
	void SecondaryAttack(void);
	void WeaponIdle(void);
	void ItemPostFrame(void);

	BOOL UseDecrement(void)
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
public:
	
	void MedigunHeal(void);
	void MedigunBeam(void);
	void StopHeal(void);
	bool PreHeal(void);
	CBasePlayer *m_pTarget;
	float m_fNextHeal;
	float m_fNextBeam;
	float m_fNextCheck;
	BOOL m_bFullyCharged;
	BOOL m_bHealing;
};

class CBonesaw : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int GetItemInfo(ItemInfo *p);
	BOOL CanDrop(void) { return FALSE; }
	BOOL Deploy(void);
	void Holster(int skiplocal);
	float GetMaxSpeed(void) { return 1; }
	int iItemSlot(void) { return WEAPON_SLOT_KNIFE; }
	void PrimaryAttack(void);

	BOOL UseDecrement(void)
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

public:
	void Swing(void);
};

class CDisguiseKit : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int GetItemInfo(ItemInfo *p);
	BOOL CanDrop(void) { return FALSE; }
	BOOL Deploy(void);
	void Holster(int skiplocal);
	float GetMaxSpeed(void) { return 1; }
	int iItemSlot(void) { return WEAPON_SLOT_PDA; }
	void SecondaryAttack(void);
	void WeaponIdle(void);

	BOOL UseDecrement(void)
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
};

class CWrench : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int GetItemInfo(ItemInfo *p);
	BOOL CanDrop(void) { return FALSE; }
	BOOL Deploy(void);
	void Holster(int skiplocal);
	float GetMaxSpeed(void) { return 1; }
	int iItemSlot(void) { return WEAPON_SLOT_KNIFE; }
	void PrimaryAttack(void);
	void SecondaryAttack(void);

	BOOL UseDecrement(void)
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

public:
	void Swing(void);
};

class CBuildPDA : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int GetItemInfo(ItemInfo *p);
	BOOL CanDrop(void) { return FALSE; }
	BOOL Deploy(void);
	void PrimaryAttack(void);
	void SecondaryAttack(void);
	void Holster(int skiplocal);
	BOOL CanDeploy(void);
	float GetMaxSpeed(void) { return 1; }
	int iItemSlot(void) { return WEAPON_SLOT_PDA; }
	void WeaponIdle(void);

	BOOL UseDecrement(void)
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}

public:
};

class CDestroyPDA : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	void Precache(void);
	int GetItemInfo(ItemInfo *p);
	BOOL CanDrop(void) { return FALSE; }
	BOOL Deploy(void);
	void Holster(int skiplocal);
	BOOL CanDeploy(void);
	float GetMaxSpeed(void) { return 1; }
	int iItemSlot(void) { return WEAPON_SLOT_PDA2; }
	void WeaponIdle(void);

	BOOL UseDecrement(void)
	{
#ifdef CLIENT_WEAPONS
		return TRUE;
#else
		return FALSE;
#endif
	}
};

void ViewAnglesForPlayBack(Vector &v);

#endif