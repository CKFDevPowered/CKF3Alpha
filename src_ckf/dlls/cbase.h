#ifndef CBASE_H
#define CBASE_H

#define FCAP_CUSTOMSAVE 0x00000001
#define FCAP_ACROSS_TRANSITION 0x00000002
#define FCAP_MUST_SPAWN 0x00000004
#define FCAP_DONT_SAVE 0x80000000
#define FCAP_IMPULSE_USE 0x00000008
#define FCAP_CONTINUOUS_USE 0x00000010
#define FCAP_ONOFF_USE 0x00000020
#define FCAP_DIRECTIONAL_USE 0x00000040
#define FCAP_MASTER 0x00000080
#define FCAP_FORCE_TRANSITION 0x00000080

#include "saverestore.h"
#include "schedule.h"

#ifndef MONSTEREVENT_H
#include "monsterevent.h"
#endif

#ifdef _WIN32
#define EXPORT _declspec(dllexport)
#else
#define EXPORT
#endif

extern "C" EXPORT int GetEntityAPI(DLL_FUNCTIONS *pFunctionTable, int interfaceVersion);
extern "C" EXPORT int GetEntityAPI2(DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion);
extern "C" EXPORT int GetNewDLLFunctions(NEW_DLL_FUNCTIONS *pFunctionTable, int *interfaceVersion);

extern int DispatchSpawn(edict_t *pent);
extern void DispatchKeyValue(edict_t *pentKeyvalue, KeyValueData *pkvd);
extern void DispatchTouch(edict_t *pentTouched, edict_t *pentOther);
extern void DispatchUse(edict_t *pentUsed, edict_t *pentOther);
extern void DispatchThink(edict_t *pent);
extern void DispatchBlocked(edict_t *pentBlocked, edict_t *pentOther);
extern void DispatchSave(edict_t *pent, SAVERESTOREDATA *pSaveData);
extern int DispatchRestore(edict_t *pent, SAVERESTOREDATA *pSaveData, int globalEntity);
extern void DispatchObjectCollsionBox(edict_t *pent);
extern void SaveWriteFields(SAVERESTOREDATA *pSaveData, const char *pname, void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount);
extern void SaveReadFields(SAVERESTOREDATA *pSaveData, const char *pname, void *pBaseData, TYPEDESCRIPTION *pFields, int fieldCount);
extern void SaveGlobalState(SAVERESTOREDATA *pSaveData);
extern void RestoreGlobalState(SAVERESTOREDATA *pSaveData);
extern void ResetGlobalState(void);
/*
typedef enum
{
	USE_OFF,
	USE_ON,
	USE_SET,
	USE_TOGGLE
}
USE_TYPE;
*/
//sohl caonima
typedef enum
{
	USE_OFF = 0,
	USE_ON = 1,
	USE_SET = 2,
	USE_TOGGLE = 3,
	USE_KILL = 4,
	USE_SPAWN = 7,  //AJH SB
	USE_SAME = 5,
	USE_NOT = 6,
} USE_TYPE;

extern char* GetStringForState( STATE state );
extern char* GetStringForUseType( USE_TYPE useType );

extern void FireTargets(const char *targetName, CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

typedef void (CBaseEntity::*BASEPTR)(void);
typedef void (CBaseEntity::*ENTITYFUNCPTR)(CBaseEntity *pOther);
typedef void (CBaseEntity::*USEPTR)(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

#define CLASS_NONE 0
#define CLASS_MOVEABLE 1
#define CLASS_PLAYER 2
#define CLASS_HUMAN_PASSIVE 3
#define CLASS_HUMAN_MILITARY 4
#define CLASS_ALIEN_MILITARY 5
#define CLASS_ALIEN_PASSIVE 6
#define CLASS_ALIEN_MONSTER 7
#define CLASS_ALIEN_PREY 8
#define CLASS_ALIEN_PREDATOR 9
#define CLASS_INSECT 10
#define CLASS_PLAYER_ALLY 11
#define CLASS_PLAYER_BIOWEAPON 12
#define CLASS_ALIEN_BIOWEAPON 13
#define CLASS_VEHICLE 14

#define CLASS_WEAPONBOX 20
#define CLASS_PROJECTILE 21
#define CLASS_BUILDABLE 22
#define CLASS_CONTROLPOINT 23

#define CLASS_BARNACLE 99
#define CLASS_CYCLERSPRITE 100

class CBaseEntity;
class CBaseMonster;
class CBasePlayerItem;
class CSquadMonster;

#define SF_NORESPAWN (1<<30)

class EHANDLE
{
public:
	edict_t *Get(void);
	edict_t *Set(edict_t *pent);

	operator int ();
	operator CBaseEntity *();

	CBaseEntity *operator = (CBaseEntity *pEntity);
	CBaseEntity *operator ->();

private:
	edict_t *m_pent;
	int m_serialnumber;
};

class CBaseEntity
{
public:
	virtual void Spawn(void) {}
	virtual void Precache(void) {}
	virtual void Restart(void) {}
	//hz add here
	virtual void KeyValue(KeyValueData* pkvd)
	{
		if (FStrEq(pkvd->szKeyName, "followentity"))
		{
			m_iFollowEntity = ALLOC_STRING(pkvd->szValue);
			pkvd->fHandled = TRUE;
		}
		else if (FStrEq(pkvd->szKeyName, "offset"))
		{
			UTIL_StringToVector( (float *)(pev->endpos), pkvd->szValue);
			pkvd->fHandled = TRUE;
		}
	}
	virtual int Save(CSave &save);
	virtual int Restore(CRestore &restore);
	virtual int ObjectCaps(void) { return FCAP_ACROSS_TRANSITION; }
	virtual void Activate(void);
	virtual void SetObjectCollisionBox(void);
	virtual int Classify(void) { return CLASS_NONE; }
	virtual void DeathNotice(entvars_t *pevChild) {}
	virtual void TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType, int iCrit);
	virtual int TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType, int iCrit);
	virtual int TakeHealth(float flHealth, int bitsDamageType);
	virtual void Killed(entvars_t *pevAttacker, int iGib);
	virtual int BloodColor(void) { return DONT_BLEED; }
	virtual void TraceBleed(float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType);
	virtual BOOL IsTriggered(CBaseEntity *pActivator) { return TRUE; }
	virtual CBaseMonster *MyMonsterPointer(void) { return NULL; }
	virtual CSquadMonster *MySquadMonsterPointer(void) { return NULL; }
	virtual int GetToggleState(void) { return TS_AT_TOP; }
	virtual void AddPoints(float score, BOOL bAllowNegativeScore) {}
	virtual void AddPointsToTeam(int score, BOOL bAllowNegativeScore) {}
	virtual BOOL AddPlayerItem(CBasePlayerItem *pItem) { return 0; }
	virtual BOOL RemovePlayerItem(CBasePlayerItem *pItem) { return 0; }
	virtual int GiveAmmo(int iAmount, char *szName, int iMax) { return -1; }
	virtual float GetDelay(void) { return 0; }
	virtual int IsMoving(void) { return pev->velocity != g_vecZero; }
	virtual void OverrideReset(void) {}
	virtual int DamageDecal(int bitsDamageType);
	virtual void SetToggleState(int state) {}
	virtual void StartSneaking(void) {}
	virtual void StopSneaking(void) {}
	virtual BOOL OnControls(entvars_t *onpev) { return FALSE; }
	virtual BOOL IsSneaking(void) { return FALSE; }
	virtual BOOL IsAlive(void) { return (pev->deadflag == DEAD_NO) && pev->health > 0; }
	virtual BOOL IsBSPModel(void) { return pev->solid == SOLID_BSP || pev->movetype == MOVETYPE_PUSHSTEP; }
	virtual BOOL ReflectGauss(void) { return IsBSPModel() && !pev->takedamage; }
	virtual BOOL HasTarget(string_t targetname) { return FStrEq(STRING(targetname), STRING(pev->targetname)); }
	virtual BOOL IsInWorld(void);
	virtual BOOL IsPlayer(void) { return FALSE; }
	virtual BOOL IsNetClient(void) { return FALSE; }
	virtual const char *TeamID(void) { return ""; }
	virtual CBaseEntity *GetNextTarget(void);
	virtual void Think(void) { if (m_pfnThink) (this->*m_pfnThink)(); }
	virtual void Touch(CBaseEntity *pOther) { if (m_pfnTouch) (this->*m_pfnTouch)(pOther); }
	virtual void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value) { if (m_pfnUse) (this->*m_pfnUse)(pActivator, pCaller, useType, value); }
	virtual void Blocked(CBaseEntity *pOther) { if (m_pfnBlocked) (this->*m_pfnBlocked)(pOther); }
	virtual CBaseEntity *Respawn(void) { return NULL; }
	virtual void UpdateOwner(void) {}
	virtual BOOL FBecomeProne(void) { return FALSE; }
	virtual Vector Center(void) { return (pev->absmax + pev->absmin) * 0.5; }
	virtual Vector EyePosition(void) { return pev->origin + pev->view_ofs; }
	virtual Vector EarPosition(void) { return pev->origin + pev->view_ofs; }
	virtual Vector BodyTarget(const Vector &posSrc) { return Center(); }
	virtual int Illumination(void) { return GETENTITYILLUM(ENT(pev)); }
	virtual BOOL FVisible(CBaseEntity *pEntity);
	virtual BOOL FVisible(const Vector &vecOrigin);
	virtual void KnockBack(Vector vecDir, float flForce){;}
	virtual void PostSpawn(void);
	//ckf3 hack
	virtual int FShouldCollide(CBaseEntity *pHit){return 1;}
	virtual int GetCriticalHit(void){ return 0; }
public:
	void EXPORT SUB_Remove(void);
	void EXPORT SUB_DoNothing(void);
	void EXPORT SUB_StartFadeOut (void);
	void EXPORT SUB_FadeOut (void);
	void EXPORT SUB_CallUseToggle(void) { Use(this, this, USE_TOGGLE, 0); }
	void SUB_UseTargets(CBaseEntity *pActivator, USE_TYPE useType, float value);

public:
	//cs15 original
	void UpdateOnRemove(void);
	int ShouldToggle(USE_TYPE useType, BOOL currentState);
	int Intersects(CBaseEntity *pOther);
	void MakeDormant(void);
	int IsDormant(void);
	BOOL IsLockedByMaster(void) { return FALSE; }
	//hz added
	Vector CKFFireBullets(Vector vecSrc, Vector vecDirShooting, float flSpread, float flDistance, int iBulletType, int iDamage, int iCritical, entvars_t *pevAttacker, int shared_rand, BOOL bApplyDamage);
	void DrawAttachTracer(int iAttachment, Vector vecDst, int iColor);
	void DrawCoordTracer(Vector vecSrc, Vector vecDst, int iColor);
public:
	static CBaseEntity *Instance(edict_t *pent) { return (CBaseEntity *)GET_PRIVATE(pent ? pent : ENT(0)); }
	static CBaseEntity *Instance(entvars_t *instpev) { return Instance(ENT(instpev)); }
	static CBaseEntity *Instance(int inst_eoffset) { return Instance(ENT(inst_eoffset)); }

	CBaseMonster *GetMonsterPointer(entvars_t *pevMonster)
	{
		CBaseEntity *pEntity = Instance(pevMonster);

		if (pEntity)
			return pEntity->MyMonsterPointer();

		return NULL;
	}

	CBaseMonster *GetMonsterPointer(edict_t *pentMonster)
	{
		CBaseEntity *pEntity = Instance(pentMonster);

		if (pEntity)
			return pEntity->MyMonsterPointer();

		return NULL;
	}

#ifdef _DEBUG
	void FunctionCheck(void *pFunction, char *name)
	{
		if (pFunction && !NAME_FOR_FUNCTION((unsigned long)(pFunction)))
			ALERT(at_error, "No EXPORT: %s:%s (%08lx)\n", STRING(pev->classname), name, (unsigned long)pFunction);
	}

	BASEPTR ThinkSet(BASEPTR func, char *name)
	{
		m_pfnThink = func;
		FunctionCheck((void *)*((int *)((char *)this + (offsetof(CBaseEntity, m_pfnThink)))), name);
		return func;
	}

	ENTITYFUNCPTR TouchSet(ENTITYFUNCPTR func, char *name)
	{
		m_pfnTouch = func;
		FunctionCheck((void *)*((int *)((char *)this + (offsetof(CBaseEntity, m_pfnTouch)))), name);
		return func;
	}

	USEPTR UseSet(USEPTR func, char *name)
	{
		m_pfnUse = func;
		FunctionCheck((void *)*((int *)((char *)this + (offsetof(CBaseEntity, m_pfnUse)))), name);
		return func;
	}

	ENTITYFUNCPTR BlockedSet(ENTITYFUNCPTR func, char *name)
	{
		m_pfnBlocked = func;
		FunctionCheck((void *)*((int *)((char *)this + (offsetof(CBaseEntity, m_pfnBlocked)))), name);
		return func;
	}
#endif

	static CBaseEntity *Create(char *szName, const Vector &vecOrigin, const Vector &vecAngles, edict_t *pentOwner = NULL);

	edict_t *edict(void) { return ENT(pev); }
	EOFFSET eoffset(void) { return OFFSET(pev); }
	int entindex(void) { return ENTINDEX(edict()); }

public:
	void *operator new(size_t stAllocateBlock, entvars_t *newpev) { return ALLOC_PRIVATE(ENT(newpev), stAllocateBlock); }

#if defined(_MSC_VER) && _MSC_VER >= 1200
	void operator delete(void *pMem, entvars_t *pev) { pev->flags |= FL_KILLME; }
#endif

public:
	static TYPEDESCRIPTION m_SaveData[];

public:
	entvars_t *pev;
	CBaseEntity *m_pGoalEnt;
	CBaseEntity *m_pLink;
	void (CBaseEntity::*m_pfnThink)(void);
	void (CBaseEntity::*m_pfnTouch)(CBaseEntity *pOther);
	void (CBaseEntity::*m_pfnUse)(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void (CBaseEntity::*m_pfnBlocked)(CBaseEntity *pOther);

	float m_flStartThrow;
	float m_flReleaseThrow;
	int m_chargeReady;
	bool m_bDisconnect;
	string_t m_iFollowEntity;
};

#ifdef _DEBUG
#define SetThink(a) ThinkSet(static_cast <void (CBaseEntity::*)(void)>(a), #a)
#define SetTouch(a) TouchSet(static_cast <void (CBaseEntity::*)(CBaseEntity *)>(a), #a)
#define SetUse(a) UseSet(static_cast <void (CBaseEntity::*)(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)>(a), #a)
#define SetBlocked(a) BlockedSet(static_cast <void (CBaseEntity::*)(CBaseEntity *)>(a), #a)
#else
#define SetThink(a) m_pfnThink = static_cast<void (CBaseEntity::*)(void)>(a)
#define SetTouch(a) m_pfnTouch = static_cast<void (CBaseEntity::*)(CBaseEntity *)>(a)
#define SetUse(a) m_pfnUse = static_cast<void (CBaseEntity::*)(CBaseEntity *, CBaseEntity *, USE_TYPE, float)>(a)
#define SetBlocked(a) m_pfnBlocked = static_cast<void (CBaseEntity::*)(CBaseEntity *)>(a)
#endif

class CPointEntity : public CBaseEntity
{
public:
	void Spawn(void);
	virtual int ObjectCaps(void) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
};

typedef struct locksounds
{
	string_t sLockedSound;
	string_t sLockedSentence;
	string_t sUnlockedSound;
	string_t sUnlockedSentence;
	int iLockedSentence;
	int iUnlockedSentence;
	float flwaitSound;
	float flwaitSentence;
	BYTE bEOFLocked;
	BYTE bEOFUnlocked;
}
locksound_t;

void PlayLockSounds(entvars_t *pev, locksound_t *pls, int flocked, int fbutton);

#define MAX_MULTI_TARGETS 16
#define MS_MAX_TARGETS 32

class CMultiSource : public CPointEntity
{
public:
	void Spawn(void);
	void KeyValue(KeyValueData *pkvd);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	int ObjectCaps(void) { return (CPointEntity::ObjectCaps() | FCAP_MASTER); }
	BOOL IsTriggered(CBaseEntity *pActivator);

public:
	void EXPORT Register(void);

public:
	EHANDLE m_rgEntities[MS_MAX_TARGETS];
	int m_rgTriggered[MS_MAX_TARGETS];
	int m_iTotal;
	string_t m_globalstate;
};

class CBaseDelay : public CBaseEntity
{
public:
	void KeyValue(KeyValueData *pkvd);

public:
	void SUB_UseTargets(CBaseEntity *pActivator, USE_TYPE useType, float value);

public:
	void EXPORT DelayThink(void);

public:
	float m_flDelay;
	int m_iszKillTarget;
};

class CBaseAnimating : public CBaseDelay
{
public:
	virtual void HandleAnimEvent(MonsterEvent_t *pEvent) {}

public:
	float StudioFrameAdvance(float flInterval = 0);
	int GetSequenceFlags(void);
	int LookupActivity(int activity);
	int LookupActivityHeaviest(int activity);
	int LookupSequence(const char *label);
	void ResetSequenceInfo(void);
	void DispatchAnimEvents(float flFutureInterval = 0.1);
	float SetBoneController(int iController, float flValue);
	void InitBoneControllers(void);
	float SetBlending(int iBlender, float flValue);
	void GetBonePosition(int iBone, Vector &origin, Vector &angles);
	void GetAutomovement(Vector &origin, Vector &angles, float flInterval = 0.1);
	int FindTransition(int iEndingSequence, int iGoalSequence, int *piDir);
	void GetAttachment(int iAttachment, Vector &origin, Vector &angles);
	void SetBodygroup(int iGroup, int iValue);
	int GetBodygroup(int iGroup);
	int ExtractBbox(int sequence, float *mins, float *maxs);
	void SetSequenceBox(void);

public:
	float m_flFrameRate;
	float m_flGroundSpeed;
	float m_flLastEventCheck;
	BOOL m_fSequenceFinished;
	BOOL m_fSequenceLoops;
};

#define SF_ITEM_USE_ONLY 256

class CBaseToggle : public CBaseAnimating
{
public:
	void KeyValue(KeyValueData *pkvd);
	int GetToggleState(void) { return m_toggle_state; }
	float GetDelay(void) { return m_flWait; }

public:
	void LinearMove(Vector vecDest, float flSpeed);
	void EXPORT LinearMoveDone(void);
	void AngularMove(Vector vecDestAngle, float flSpeed);
	void EXPORT AngularMoveDone(void);
	BOOL IsLockedByMaster(void);

public:
	static float AxisValue(int flags, const Vector &angles);
	static void AxisDir(entvars_t *pev);
	static float AxisDelta(int flags, const Vector &angle1, const Vector &angle2);

public:
	TOGGLE_STATE m_toggle_state;
	float m_flActivateFinished;
	float m_flMoveDistance;
	float m_flWait;
	float m_flLip;
	float m_flTWidth;
	float m_flTLength;
	Vector m_vecPosition1;
	Vector m_vecPosition2;
	Vector m_vecAngle1;
	Vector m_vecAngle2;
	int m_cTriggersLeft;
	float m_flHeight;
	EHANDLE m_hActivator;
	void (CBaseToggle::*m_pfnCallWhenMoveDone)(void);
	Vector m_vecFinalDest;
	Vector m_vecFinalAngle;
	int m_bitsDamageInflict;
	string_t m_sMaster;
};

#define SetMoveDone(a) m_pfnCallWhenMoveDone = static_cast<void (CBaseToggle::*)(void)>(a)

#define GIB_HEALTH_VALUE -30

#define ROUTE_SIZE 8
#define MAX_OLD_ENEMIES 4

#define bits_CAP_DUCK (1<<0)
#define bits_CAP_JUMP (1<<1)
#define bits_CAP_STRAFE (1<<2)
#define bits_CAP_SQUAD (1<<3)
#define bits_CAP_SWIM (1<<4)
#define bits_CAP_CLIMB (1<<5)
#define bits_CAP_USE (1<<6)
#define bits_CAP_HEAR (1<<7)
#define bits_CAP_AUTO_DOORS (1<<8)
#define bits_CAP_OPEN_DOORS (1<<9)
#define bits_CAP_TURN_HEAD (1<<10)
#define bits_CAP_RANGE_ATTACK1 (1<<11)
#define bits_CAP_RANGE_ATTACK2 (1<<12)
#define bits_CAP_MELEE_ATTACK1 (1<<13)
#define bits_CAP_MELEE_ATTACK2 (1<<14)
#define bits_CAP_FLY (1<<15)
#define bits_CAP_DOORS_GROUP (bits_CAP_USE | bits_CAP_AUTO_DOORS | bits_CAP_OPEN_DOORS)

#define DMG_GENERIC 0
#define DMG_CRUSH (1<<0)
#define DMG_BULLET (1<<1)
#define DMG_SLASH (1<<2)
#define DMG_BURN (1<<3)
#define DMG_FREEZE (1<<4)
#define DMG_FALL (1<<5)
#define DMG_BLAST (1<<6)
#define DMG_CLUB (1<<7)
#define DMG_SHOCK (1<<8)
#define DMG_BACKSTAB (1<<9)//fuckup??
#define DMG_ENERGYBEAM (1<<10)
#define DMG_NEVERGIB (1<<12)
#define DMG_ALWAYSGIB (1<<13)
#define DMG_DROWN (1<<14)
#define DMG_TIMEBASED (~(0x3FFF))

#define DMG_PARALYZE (1<<15)
#define DMG_MINICRIT (1<<16)//wtf??
#define DMG_SAPPER (1<<17)
#define DMG_RADIATION (1<<18)
#define DMG_DROWNRECOVER (1<<19)
#define DMG_ACID (1<<20)
#define DMG_FLAME (1<<21)
#define DMG_NOSELFDMG (1<<22)//No Self Damage
#define DMG_RANGE (1<<23)//TF2 Range influenced
#define DMG_SHAKE (1<<24)//Shake Screen
#define DMG_GIB_CORPSE (DMG_CRUSH | DMG_FALL | DMG_BLAST | DMG_CLUB)
#define DMG_SHOWNHUD (DMG_ACID | DMG_FREEZE | DMG_DROWN | DMG_BURN | DMG_RADIATION | DMG_SHOCK)

#define PARALYZE_DURATION 2
#define PARALYZE_DAMAGE 1.0

#define NERVEGAS_DURATION 2
#define NERVEGAS_DAMAGE 5.0

#define POISON_DURATION 5
#define POISON_DAMAGE 2.0

#define RADIATION_DURATION 2
#define RADIATION_DAMAGE 1.0

#define ACID_DURATION 2
#define ACID_DAMAGE 5.0

#define SLOWBURN_DURATION 2
#define SLOWBURN_DAMAGE 1.0

#define SLOWFREEZE_DURATION 2
#define SLOWFREEZE_DAMAGE 1.0

#define itbd_Paralyze 0
#define itbd_NerveGas 1
#define itbd_Poison 2
#define itbd_Radiation 3
#define itbd_DrownRecover 4
#define itbd_Acid 5
#define itbd_SlowBurn 6
#define itbd_SlowFreeze 7
#define CDMG_TIMEBASED 8

#define GIB_NORMAL 0
#define GIB_NEVER 1
#define GIB_ALWAYS 2

class CBaseMonster;
class CCineMonster;
class CSound;

#include "basemonster.h"

char *ButtonSound(int sound);

class CBaseButton : public CBaseToggle
{
public:
	void Spawn(void);
	void Precache(void);
	void KeyValue(KeyValueData* pkvd);
	int ObjectCaps(void) { return (CBaseToggle:: ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | (pev->takedamage ? 0 : FCAP_IMPULSE_USE); }
	int TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType, int iCrit);

public:
	void RotSpawn(void);
	void ButtonActivate(void);
	void SparkSoundCache(void);

	void EXPORT ButtonShot(void);
	void EXPORT ButtonTouch(CBaseEntity *pOther);
	void EXPORT ButtonSpark(void);
	void EXPORT TriggerAndWait(void);
	void EXPORT ButtonReturn(void);
	void EXPORT ButtonBackHome(void);
	void EXPORT ButtonUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void EXPORT ButtonUse_IgnorePlayer( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );//sohl sb
	enum BUTTON_CODE { BUTTON_NOTHING, BUTTON_ACTIVATE, BUTTON_RETURN };
	BUTTON_CODE ButtonResponseToTouch(void);

public:
	BOOL m_fStayPushed;
	BOOL m_fRotating;
	string_t m_strChangeTarget;
	locksound_t m_ls;
	BYTE m_bLockedSound;
	BYTE m_bLockedSentence;
	BYTE m_bUnlockedSound;
	BYTE m_bUnlockedSentence;
	int m_sounds;
};

template <class T> T *GetClassPtr(T *a)
{
	entvars_t *pev = (entvars_t *)a;

	if (pev == NULL)
		pev = VARS(CREATE_ENTITY());

	a = (T *)GET_PRIVATE(ENT(pev));

	if (a == NULL)
	{
		a = new(pev) T;
		a->pev = pev;
	}

	return a;
}

class CWorld : public CBaseEntity
{
public:
	void Spawn(void);
	void Precache(void);
	void KeyValue(KeyValueData *pkvd);
};

class CClientFog : public CBaseEntity
{
public:
	void Spawn(void);
	void KeyValue(KeyValueData *pkvd);

public:
	int m_iStartDist, m_iEndDist;
	float m_fDensity;
};

extern CWorld *g_pWorld;

class CNullEntity : public CBaseEntity
{
public:
	void Spawn( void );
};

class CBaseDMStart : public CPointEntity
{
public:
	void		KeyValue( KeyValueData *pkvd );
	void		Spawn(void);
	void		Restart(void);
public:
	int			m_iDisabled;
};

#endif