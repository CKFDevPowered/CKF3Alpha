#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "animation.h"
#include "weapons.h"
#include "player.h"

#define TEMP_FOR_SCREEN_SHOTS
#ifdef TEMP_FOR_SCREEN_SHOTS

class CCycler : public CBaseMonster
{
public:
	void GenericCyclerSpawn(char *szModel, Vector vecMin, Vector vecMax);
	int ObjectCaps(void) { return (CBaseEntity::ObjectCaps() | FCAP_IMPULSE_USE); }
	int TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType);
	void Spawn(void);
	void Think(void);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	BOOL IsAlive(void) { return FALSE; }
	int Save(CSave &save);
	int Restore(CRestore &restore);

public:
	static TYPEDESCRIPTION m_SaveData[];

public:
	int m_animate;
};

TYPEDESCRIPTION CCycler::m_SaveData[] =
{
	DEFINE_FIELD(CCycler, m_animate, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CCycler, CBaseMonster);

class CGenericCycler : public CCycler
{
public:
	void Spawn(void) { GenericCyclerSpawn((char *)STRING(pev->model), Vector(-16, -16, 0), Vector(16, 16, 72)); }
};

LINK_ENTITY_TO_CLASS(cycler, CGenericCycler);

class CCyclerProbe : public CCycler
{
public:
	void Spawn(void);
};

LINK_ENTITY_TO_CLASS(cycler_prdroid, CCyclerProbe);

void CCyclerProbe::Spawn(void)
{
	pev->origin = pev->origin + Vector(0, 0, 16);
	GenericCyclerSpawn("models/prdroid.mdl", Vector(-16, -16, -16), Vector(16, 16, 16));
}

void CCycler::GenericCyclerSpawn(char *szModel, Vector vecMin, Vector vecMax)
{
	if (!szModel || !*szModel)
	{
		ALERT(at_error, "cycler at %.0f %.0f %0.f missing modelname", pev->origin.x, pev->origin.y, pev->origin.z);
		REMOVE_ENTITY(ENT(pev));
		return;
	}

	pev->classname = MAKE_STRING("cycler");
	PRECACHE_MODEL(szModel);
	SET_MODEL(ENT(pev), szModel);
	CCycler::Spawn();
	UTIL_SetSize(pev, vecMin, vecMax);
}

void CCycler::Spawn(void)
{
	InitBoneControllers();
	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_NONE;
	pev->takedamage = DAMAGE_YES;
	pev->effects = 0;
	pev->health = 80000;
	pev->yaw_speed = 5;
	pev->ideal_yaw = pev->angles.y;
	ChangeYaw(360);

	m_flFrameRate = 75;
	m_flGroundSpeed = 0;
	pev->nextthink += 1;

	ResetSequenceInfo();

	if (pev->sequence != 0 || pev->frame != 0)
	{
		m_animate = 0;
		pev->framerate = 0;
	}
	else
		m_animate = 1;
}

void CCycler::Think(void)
{
	pev->nextthink = gpGlobals->time + 0.1;

	if (m_animate)
		StudioFrameAdvance();

	if (m_fSequenceFinished && !m_fSequenceLoops)
	{
		pev->animtime = gpGlobals->time;
		pev->framerate = 1;
		m_fSequenceFinished = FALSE;
		m_flLastEventCheck = gpGlobals->time;
		pev->frame = 0;

		if (!m_animate)
			pev->framerate = 0;
	}
}

void CCycler::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	m_animate = !m_animate;

	if (m_animate)
		pev->framerate = 1;
	else
		pev->framerate = 0;
}

int CCycler::TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType)
{
	if (m_animate)
	{
		pev->sequence++;
		ResetSequenceInfo();

		if (m_flFrameRate == 0)
		{
			pev->sequence = 0;
			ResetSequenceInfo();
		}

		pev->frame = 0;
	}
	else
	{
		pev->framerate = 1.0;
		StudioFrameAdvance(0.1);
		pev->framerate = 0;
		ALERT(at_console, "sequence: %d, frame %.0f\n", pev->sequence, pev->frame);
	}

	return 0;
}

#endif

class CCyclerSprite : public CBaseEntity
{
public:
	void Spawn(void);
	void Think(void);
	int Classify(void) {return CLASS_CYCLERSPRITE;}
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	int ObjectCaps(void) { return (CBaseEntity::ObjectCaps() | FCAP_DONT_SAVE | FCAP_IMPULSE_USE); }
	int TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType);
	void Animate(float frames);

public:
	inline int ShouldAnimate(void) { return m_animate && m_maxFrame > 1.0; }

	int m_animate;
	float m_lastTime;
	float m_maxFrame;
};

LINK_ENTITY_TO_CLASS(cycler_sprite, CCyclerSprite);

void CCyclerSprite::Spawn(void)
{
	pev->solid = SOLID_NOT;//SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_NONE;
	pev->takedamage = DAMAGE_NO;//DAMAGE_YES;
	pev->effects = 0;
	pev->frame = 0;
	pev->nextthink = gpGlobals->time + 0.1;
	m_animate = 1;
	m_lastTime = gpGlobals->time;

	PRECACHE_MODEL((char *)STRING(pev->model));
	SET_MODEL(ENT(pev), STRING(pev->model));
	m_maxFrame = (float)MODEL_FRAMES(pev->modelindex) - 1;

	ALERT(at_console, "cycler_sprite->pev->flags = %d\n", pev->flags);
	ALERT(at_console, "cycler_sprite->pev->effects = %d\n", pev->effects);
}

void CCyclerSprite::Think(void)
{
	if (ShouldAnimate())
		Animate(pev->framerate * (gpGlobals->time - m_lastTime));

	pev->nextthink = gpGlobals->time + 0.1;
	m_lastTime = gpGlobals->time;
}

void CCyclerSprite::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	m_animate = !m_animate;
	ALERT(at_console, "Sprite: %s\n", STRING(pev->model));
}

int CCyclerSprite::TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType)
{
	if (m_maxFrame > 1)
		Animate(1);

	return 1;
}

void CCyclerSprite::Animate(float frames)
{
	pev->frame += frames;

	if (m_maxFrame > 0)
		pev->frame = fmod(pev->frame, m_maxFrame);
}

class CWeaponCycler : public CBasePlayerWeapon
{
public:
	void Spawn(void);
	int iItemSlot(void) { return 1; }
	int GetItemInfo(ItemInfo *p) { return 0; }
	void PrimaryAttack(void);
	void SecondaryAttack(void);
	BOOL Deploy(void);
	void Holster(int skiplocal = 0);

public:
	int m_iszModel;
	int m_iModel;
};

LINK_ENTITY_TO_CLASS(cycler_weapon, CWeaponCycler);

void CWeaponCycler::Spawn(void)
{
	pev->solid = SOLID_SLIDEBOX;
	pev->movetype = MOVETYPE_NONE;

	PRECACHE_MODEL((char *)STRING(pev->model));
	SET_MODEL(ENT(pev), STRING(pev->model));

	m_iszModel = pev->model;
	m_iModel = pev->modelindex;

	UTIL_SetOrigin(pev, pev->origin);
	UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 16));
	SetTouch(&CBasePlayerItem::DefaultTouch);
}

BOOL CWeaponCycler::Deploy(void)
{
	m_pPlayer->pev->viewmodel = m_iszModel;
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 1;

	SendWeaponAnim(0);
	m_iClip = 0;
	return TRUE;
}

void CWeaponCycler::Holster(int skiplocal)
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;
}

void CWeaponCycler::PrimaryAttack()
{
	SendWeaponAnim(pev->sequence);
	m_flNextPrimaryAttack = gpGlobals->time + 0.3;
}

void CWeaponCycler::SecondaryAttack(void)
{
	pev->sequence = (pev->sequence + 1) % 8;
	pev->modelindex = m_iModel;
	void *pmodel = GET_MODEL_PTR(ENT(pev));

	float flFrameRate, flGroundSpeed;
	GetSequenceInfo(pmodel, pev, &flFrameRate, &flGroundSpeed);
	pev->modelindex = 0;

	if (flFrameRate == 0)
		pev->sequence = 0;

	SendWeaponAnim(pev->sequence);
	m_flNextSecondaryAttack = gpGlobals->time + 0.3;
}

class CWreckage : public CBaseMonster
{
	int Save(CSave &save);
	int Restore(CRestore &restore);
	void Spawn(void);
	void Precache(void);
	void Think(void);

public:
	static TYPEDESCRIPTION m_SaveData[];

public:
	int m_flStartTime;
};

TYPEDESCRIPTION CWreckage::m_SaveData[] =
{
	DEFINE_FIELD(CWreckage, m_flStartTime, FIELD_TIME),
};

IMPLEMENT_SAVERESTORE(CWreckage, CBaseMonster);
LINK_ENTITY_TO_CLASS(cycler_wreckage, CWreckage);

void CWreckage::Spawn(void)
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->takedamage = 0;
	pev->effects = 0;
	pev->frame = 0;
	pev->nextthink = gpGlobals->time + 0.1;

	if (pev->model)
	{
		PRECACHE_MODEL((char *)STRING(pev->model));
		SET_MODEL(ENT(pev), STRING(pev->model));
	}

	m_flStartTime = (int)(gpGlobals->time);
}

void CWreckage::Precache(void)
{
	if (pev->model)
		PRECACHE_MODEL((char *)STRING(pev->model));
}

void CWreckage::Think(void)
{
	StudioFrameAdvance();
	pev->nextthink = gpGlobals->time + 0.2;

	if (pev->dmgtime)
	{
		if (pev->dmgtime < gpGlobals->time)
		{
			UTIL_Remove(this);
			return;
		}
		else if (RANDOM_FLOAT(0, pev->dmgtime - m_flStartTime) > pev->dmgtime - gpGlobals->time)
			return;
	}

	Vector VecSrc;
	VecSrc.x = RANDOM_FLOAT(pev->absmin.x, pev->absmax.x);
	VecSrc.y = RANDOM_FLOAT(pev->absmin.y, pev->absmax.y);
	VecSrc.z = RANDOM_FLOAT(pev->absmin.z, pev->absmax.z);

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, VecSrc);
	WRITE_BYTE(TE_SMOKE);
	WRITE_COORD(VecSrc.x);
	WRITE_COORD(VecSrc.y);
	WRITE_COORD(VecSrc.z);
	WRITE_SHORT(g_sModelIndexSmoke);
	WRITE_BYTE(RANDOM_LONG(0, 49) + 50);
	WRITE_BYTE(RANDOM_LONG(0, 3) + 8);
	MESSAGE_END();
}

class CFloatText : public CBaseEntity
{
public:
	int ObjectCaps(void) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	void Spawn(void);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void KeyValue( KeyValueData *pkvd );
	void SendFloatText(entvars_t *pevPlayer);
public:
	float m_fLife;
	float m_fMaxLife;
	int m_iFTFlags;
	int m_iSize;
	int m_iMaxDist;
	int m_iFadeDist;	
	Vector m_vecVelocity;
};

LINK_ENTITY_TO_CLASS(game_floattext, CFloatText);

#define SF_FLOATTEXT_TRIGGER_ONLY 1
#define SF_FLOATTEXT_LIFE 2

#define FTEXT_ISCONTEXT (1<<0)
#define FTEXT_PERSPECTIVE (1<<1)

void CFloatText::Spawn(void)
{
	m_iFTFlags = 0;
	pev->iuser1 = 0;
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	pev->takedamage = DAMAGE_NO;
}

void CFloatText::KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "life"))
	{
		m_fMaxLife = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "vel"))
	{
		UTIL_StringToVector(m_vecVelocity, pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "size"))
	{
		m_iSize = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "text"))
	{
		pev->message = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue( pkvd );
}

void CFloatText::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	pev->iuser1 = 1 - pev->iuser1;

	if (pev->iuser1)
	{
		if((pev->spawnflags & SF_FLOATTEXT_LIFE) && m_vecVelocity.Length() == 0)
			m_fMaxLife = 0;

		m_fLife = gpGlobals->time + m_fMaxLife;

		if(pActivator->IsPlayer() && (pev->spawnflags & SF_FLOATTEXT_TRIGGER_ONLY))
		{
			SendFloatText(pActivator->pev);
		}
		else
		{
			SendFloatText(NULL);
		}
	}
}

void CFloatText::SendFloatText(entvars_t *pevPlayer)
{
	/*int iAimEnt = 0;
	if(pev->aiment) iAimEnt = ENTINDEX(pev->aiment);

	if(pevPlayer)
	{
		MESSAGE_BEGIN(MSG_ONE, gmsgFloatText, NULL, pevPlayer);
	}
	else
	{
		MESSAGE_BEGIN(MSG_ALL, gmsgFloatText);
	}
	WRITE_COORD((iAimEnt) ? pev->endpos.x:pev->origin.x);
	WRITE_COORD((iAimEnt) ? pev->endpos.y:pev->origin.y);
	WRITE_COORD((iAimEnt) ? pev->endpos.z:pev->origin.z);
	WRITE_COORD(m_vecVelocity.x);
	WRITE_COORD(m_vecVelocity.y);
	WRITE_COORD(m_vecVelocity.z);
	WRITE_BYTE((int)pev->rendercolor.x);
	WRITE_BYTE((int)pev->rendercolor.y);
	WRITE_BYTE((int)pev->rendercolor.z);
	WRITE_BYTE(m_iFTFlags);
	WRITE_BYTE(m_iSize);
	WRITE_SHORT(m_iMaxDist);
	WRITE_SHORT(m_iFadeDist);		
	WRITE_SHORT(int(m_fMaxLife*10));
	WRITE_SHORT(iAimEnt);
	WRITE_STRING(STRING(pev->message));
	MESSAGE_END();*/
}