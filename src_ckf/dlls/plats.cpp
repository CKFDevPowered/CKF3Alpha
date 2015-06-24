#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "trains.h"
#include "saverestore.h"

static void PlatSpawnInsideTrigger(entvars_t *pevPlatform);

#define SF_PLAT_TOGGLE 0x0001

class CBasePlatTrain : public CBaseToggle
{
public:
	int ObjectCaps(void) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	void KeyValue(KeyValueData *pkvd);
	void Precache(void);
	BOOL IsTogglePlat(void) { return (pev->spawnflags & SF_PLAT_TOGGLE) ? TRUE : FALSE; }

public:
	BYTE m_bMoveSnd;
	BYTE m_bStopSnd;
	float m_volume;
};

void CBasePlatTrain::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "lip"))
	{
		m_flLip = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "height"))
	{
		m_flHeight = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "rotation"))
	{
		m_vecFinalAngle.x = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "movesnd"))
	{
		m_bMoveSnd = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "stopsnd"))
	{
		m_bStopSnd = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "volume"))
	{
		m_volume = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "custommovesnd"))
	{
		pev->noise = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "customstopsnd"))
	{
		pev->noise1 = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseToggle::KeyValue(pkvd);
}

#define noiseMoving noise
#define noiseArrived noise1

void CBasePlatTrain::Precache(void)
{
	switch (m_bMoveSnd)
	{
		case 0: pev->noiseMoving = MAKE_STRING("common/null.wav"); break;

		case 1:
		{
			PRECACHE_SOUND("plats/bigmove1.wav");
			pev->noiseMoving = MAKE_STRING("plats/bigmove1.wav");
			break;
		}

		case 2:
		{
			PRECACHE_SOUND("plats/bigmove2.wav");
			pev->noiseMoving = MAKE_STRING("plats/bigmove2.wav");
			break;
		}

		case 3:
		{
			PRECACHE_SOUND("plats/elevmove1.wav");
			pev->noiseMoving = MAKE_STRING("plats/elevmove1.wav");
			break;
		}

		case 4:
		{
			PRECACHE_SOUND("plats/elevmove2.wav");
			pev->noiseMoving = MAKE_STRING("plats/elevmove2.wav");
			break;
		}

		case 5:
		{
			PRECACHE_SOUND("plats/elevmove3.wav");
			pev->noiseMoving = MAKE_STRING("plats/elevmove3.wav");
			break;
		}

		case 6:
		{
			PRECACHE_SOUND("plats/freightmove1.wav");
			pev->noiseMoving = MAKE_STRING("plats/freightmove1.wav");
			break;
		}

		case 7:
		{
			PRECACHE_SOUND("plats/freightmove2.wav");
			pev->noiseMoving = MAKE_STRING("plats/freightmove2.wav");
			break;
		}

		case 8:
		{
			PRECACHE_SOUND("plats/heavymove1.wav");
			pev->noiseMoving = MAKE_STRING("plats/heavymove1.wav");
			break;
		}

		case 9:
		{
			PRECACHE_SOUND("plats/rackmove1.wav");
			pev->noiseMoving = MAKE_STRING("plats/rackmove1.wav");
			break;
		}

		case 10:
		{
			PRECACHE_SOUND("plats/railmove1.wav");
			pev->noiseMoving = MAKE_STRING("plats/railmove1.wav");
			break;
		}

		case 11:
		{
			PRECACHE_SOUND("plats/squeekmove1.wav");
			pev->noiseMoving = MAKE_STRING("plats/squeekmove1.wav");
			break;
		}

		case 12:
		{
			PRECACHE_SOUND("plats/talkmove1.wav");
			pev->noiseMoving = MAKE_STRING("plats/talkmove1.wav");
			break;
		}

		case 13:
		{
			PRECACHE_SOUND("plats/talkmove2.wav");
			pev->noiseMoving = MAKE_STRING("plats/talkmove2.wav");
			break;
		}

		default: pev->noiseMoving = MAKE_STRING("common/null.wav"); break;
	}

	switch (m_bStopSnd)
	{
		case 0: pev->noiseArrived = MAKE_STRING("common/null.wav"); break;

		case 1:
		{
			PRECACHE_SOUND("plats/bigstop1.wav");
			pev->noiseArrived = MAKE_STRING("plats/bigstop1.wav");
			break;
		}

		case 2:
		{
			PRECACHE_SOUND("plats/bigstop2.wav");
			pev->noiseArrived = MAKE_STRING("plats/bigstop2.wav");
			break;
		}

		case 3:
		{
			PRECACHE_SOUND("plats/freightstop1.wav");
			pev->noiseArrived = MAKE_STRING("plats/freightstop1.wav");
			break;
		}

		case 4:
		{
			PRECACHE_SOUND("plats/heavystop2.wav");
			pev->noiseArrived = MAKE_STRING("plats/heavystop2.wav");
			break;
		}

		case 5:
		{
			PRECACHE_SOUND("plats/rackstop1.wav");
			pev->noiseArrived = MAKE_STRING("plats/rackstop1.wav");
			break;
		}

		case 6:
		{
			PRECACHE_SOUND("plats/railstop1.wav");
			pev->noiseArrived = MAKE_STRING("plats/railstop1.wav");
			break;
		}

		case 7:
		{
			PRECACHE_SOUND("plats/squeekstop1.wav");
			pev->noiseArrived = MAKE_STRING("plats/squeekstop1.wav");
			break;
		}

		case 8:
		{
			PRECACHE_SOUND("plats/talkstop1.wav");
			pev->noiseArrived = MAKE_STRING("plats/talkstop1.wav");
			break;
		}

		default: pev->noiseArrived = MAKE_STRING("common/null.wav"); break;
	}
}

#define noiseMovement noise
#define noiseStopMoving noise1

class CFuncPlat : public CBasePlatTrain
{
public:
	virtual void Spawn(void);
	virtual void Precache(void);
	virtual void Blocked(CBaseEntity *pOther);
	virtual void GoUp(void);
	virtual void GoDown(void);
	virtual void HitTop(void);
	virtual void HitBottom(void);

public:
	void Setup(void);

public:
	void EXPORT PlatUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void EXPORT CallGoDown(void) { GoDown(); }
	void EXPORT CallHitTop(void) { HitTop(); }
	void EXPORT CallHitBottom(void) { HitBottom(); }
};

LINK_ENTITY_TO_CLASS(func_plat, CFuncPlat);

class CPlatTrigger : public CBaseEntity
{
public:
	int ObjectCaps(void) { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_DONT_SAVE; }
	void SpawnInsideTrigger(CFuncPlat *pPlatform);
	void Touch(CBaseEntity *pOther);

public:
	CFuncPlat *m_pPlatform;
};

void CFuncPlat::Setup(void)
{
	if (m_flTLength == 0)
		m_flTLength = 80;

	if (m_flTWidth == 0)
		m_flTWidth = 10;

	pev->angles = g_vecZero;
	pev->solid = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;

	UTIL_SetOrigin(pev, pev->origin);
	UTIL_SetSize(pev, pev->mins, pev->maxs);
	SET_MODEL(ENT(pev), STRING(pev->model));

	m_vecPosition1 = pev->origin;
	m_vecPosition2 = pev->origin;

	if (m_flHeight != 0)
		m_vecPosition2.z = pev->origin.z - m_flHeight;
	else
		m_vecPosition2.z = pev->origin.z - pev->size.z + 8;

	if (pev->speed == 0)
		pev->speed = 150;

	if (m_volume == 0)
		m_volume = 0.85;
}

void CFuncPlat::Precache(void)
{
	CBasePlatTrain::Precache();

	if (!IsTogglePlat())
		PlatSpawnInsideTrigger(pev);
}

void CFuncPlat::Spawn(void)
{
	Setup();
	Precache();

	if (!FStringNull(pev->targetname))
	{
		UTIL_SetOrigin(pev, m_vecPosition1);
		m_toggle_state = TS_AT_TOP;
		SetUse(&CFuncPlat::PlatUse);
	}
	else
	{
		UTIL_SetOrigin(pev, m_vecPosition2);
		m_toggle_state = TS_AT_BOTTOM;
	}
}

static void PlatSpawnInsideTrigger(entvars_t *pevPlatform)
{
	GetClassPtr((CPlatTrigger *)NULL)->SpawnInsideTrigger(GetClassPtr((CFuncPlat *)pevPlatform));
}

void CPlatTrigger::SpawnInsideTrigger(CFuncPlat *pPlatform)
{
	m_pPlatform = pPlatform;
	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_NONE;
	pev->origin = pPlatform->pev->origin;

	Vector vecTMin = m_pPlatform->pev->mins + Vector(25, 25, 0);
	Vector vecTMax = m_pPlatform->pev->maxs + Vector(25, 25, 8);
	vecTMin.z = vecTMax.z - (m_pPlatform->m_vecPosition1.z - m_pPlatform->m_vecPosition2.z + 8);

	if (m_pPlatform->pev->size.x <= 50)
	{
		vecTMin.x = (m_pPlatform->pev->mins.x + m_pPlatform->pev->maxs.x) / 2;
		vecTMax.x = vecTMin.x + 1;
	}

	if (m_pPlatform->pev->size.y <= 50)
	{
		vecTMin.y = (m_pPlatform->pev->mins.y + m_pPlatform->pev->maxs.y) / 2;
		vecTMax.y = vecTMin.y + 1;
	}

	UTIL_SetSize(pev, vecTMin, vecTMax);
}

void CPlatTrigger::Touch(CBaseEntity *pOther)
{
	entvars_t *pevToucher = pOther->pev;

	if (!FClassnameIs(pevToucher, "player"))
		return;

	if (!pOther->IsAlive())
		return;

	if (m_pPlatform->m_toggle_state == TS_AT_BOTTOM)
		m_pPlatform->GoUp();
	else if (m_pPlatform->m_toggle_state == TS_AT_TOP)
		m_pPlatform->pev->nextthink = m_pPlatform->pev->ltime + 1;
}

void CFuncPlat::PlatUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (IsTogglePlat())
	{
		BOOL on = (m_toggle_state == TS_AT_BOTTOM) ? TRUE : FALSE;

		if (!ShouldToggle(useType, on))
			return;

		if (m_toggle_state == TS_AT_TOP)
			GoDown();
		else if (m_toggle_state == TS_AT_BOTTOM)
			GoUp();
	}
	else
	{
		SetUse(NULL);

		if (m_toggle_state == TS_AT_TOP)
			GoDown();
	}
}

void CFuncPlat::GoDown(void)
{
	if (pev->noiseMovement)
		EMIT_SOUND(ENT(pev), CHAN_STATIC, (char *)STRING(pev->noiseMovement), m_volume, ATTN_NORM);

	m_toggle_state = TS_GOING_DOWN;
	SetMoveDone(&CFuncPlat::HitBottom);
	LinearMove(m_vecPosition2, pev->speed);
}

void CFuncPlat::HitBottom(void)
{
	if (pev->noiseMovement)
		STOP_SOUND(ENT(pev), CHAN_STATIC, (char *)STRING(pev->noiseMovement));

	if (pev->noiseStopMoving)
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, (char *)STRING(pev->noiseStopMoving), m_volume, ATTN_NORM);

	m_toggle_state = TS_AT_BOTTOM;
}

void CFuncPlat::GoUp(void)
{
	if (pev->noiseMovement)
		EMIT_SOUND(ENT(pev), CHAN_STATIC, (char *)STRING(pev->noiseMovement), m_volume, ATTN_NORM);

	m_toggle_state = TS_GOING_UP;
	SetMoveDone(&CFuncPlat::HitTop);
	LinearMove(m_vecPosition1, pev->speed);
}

void CFuncPlat::HitTop(void)
{
	if (pev->noiseMovement)
		STOP_SOUND(ENT(pev), CHAN_STATIC, (char *)STRING(pev->noiseMovement));

	if (pev->noiseStopMoving)
		EMIT_SOUND(ENT(pev), CHAN_WEAPON, (char *)STRING(pev->noiseStopMoving), m_volume, ATTN_NORM);

	m_toggle_state = TS_AT_TOP;

	if (!IsTogglePlat())
	{
		SetThink(&CFuncPlat::CallGoDown);
		pev->nextthink = pev->ltime + 3;
	}
}

void CFuncPlat::Blocked(CBaseEntity *pOther)
{
	ALERT(at_aiconsole, "%s Blocked by %s\n", STRING(pev->classname), STRING(pOther->pev->classname));

	//pOther->TakeDamage(pev, pev, 1, DMG_CRUSH);
	//sohl
	if (m_hActivator)
		pOther->TakeDamage( pev, m_hActivator->pev, 1, DMG_CRUSH, 0 );	//AJH Attribute damage to he who switched me.
	else
		pOther->TakeDamage( pev, pev,1, DMG_CRUSH, 0 );

	if (pev->noiseMovement)
		STOP_SOUND(ENT(pev), CHAN_STATIC, (char *)STRING(pev->noiseMovement));

	if (m_toggle_state == TS_GOING_UP)
		GoDown();
	else if (m_toggle_state == TS_GOING_DOWN)
		GoUp();
}

class CFuncPlatRot : public CFuncPlat
{
public:
	void Spawn(void);
	void SetupRotation(void);
	void GoUp(void);
	void GoDown(void);
	void HitTop(void);
	void HitBottom(void);
	void RotMove(Vector &destAngle, float time);

public:
	Vector m_end, m_start;
};

LINK_ENTITY_TO_CLASS(func_platrot, CFuncPlatRot);

void CFuncPlatRot::SetupRotation(void)
{
	if (m_vecFinalAngle.x != 0)
	{
		CBaseToggle::AxisDir(pev);
		m_start = pev->angles;
		m_end = pev->angles + pev->movedir * m_vecFinalAngle.x;
	}
	else
	{
		m_start = g_vecZero;
		m_end = g_vecZero;
	}

	if (!FStringNull(pev->targetname))
		pev->angles = m_end;
}

void CFuncPlatRot::Spawn(void)
{
	CFuncPlat::Spawn();
	SetupRotation();
}

void CFuncPlatRot::GoDown(void)
{
	CFuncPlat::GoDown();
	RotMove(m_start, pev->nextthink - pev->ltime);
}

void CFuncPlatRot::HitBottom(void)
{
	CFuncPlat::HitBottom();
	pev->avelocity = g_vecZero;
	pev->angles = m_start;
}

void CFuncPlatRot::GoUp(void)
{
	CFuncPlat::GoUp();
	RotMove(m_end, pev->nextthink - pev->ltime);
}

void CFuncPlatRot::HitTop(void)
{
	CFuncPlat::HitTop();
	pev->avelocity = g_vecZero;
	pev->angles = m_end;
}

void CFuncPlatRot::RotMove(Vector &destAngle, float time)
{
	Vector vecDestDelta = destAngle - pev->angles;

	if (time >= 0.1)
	{
		pev->avelocity = vecDestDelta / time;
	}
	else
	{
		pev->avelocity = vecDestDelta;
		pev->nextthink = pev->ltime + 1;
	}
}

class CFuncTrain : public CBasePlatTrain
{
public:
	void Spawn(void);
	void Restart(void);
	void Precache(void);
	void Activate(void);
	void OverrideReset(void);
	void Blocked(CBaseEntity *pOther);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void KeyValue(KeyValueData *pkvd);

public:
	void EXPORT Wait(void);
	void EXPORT Next(void);

public:
	Vector m_vecSaveOrigin;
	entvars_t *m_pevSaveTarget;
	entvars_t *m_pevCurrentTarget;
	int m_sounds;
	BOOL m_activated;
};

LINK_ENTITY_TO_CLASS(func_train, CFuncTrain);

void CFuncTrain::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "sounds"))
	{
		m_sounds = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBasePlatTrain::KeyValue(pkvd);
}

void CFuncTrain::Blocked(CBaseEntity *pOther)
{
	if (gpGlobals->time < m_flActivateFinished)
		return;

	m_flActivateFinished = gpGlobals->time + 0.5;
	//sohl
	if (pev->dmg)
		if (m_hActivator)
			pOther->TakeDamage( pev, m_hActivator->pev, pev->dmg, DMG_CRUSH, 0 );
		else
			pOther->TakeDamage( pev, pev, pev->dmg, DMG_CRUSH, 0 );
	//pOther->TakeDamage(pev, pev, pev->dmg, DMG_CRUSH);
}

void CFuncTrain::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (pev->spawnflags & SF_TRAIN_WAIT_RETRIGGER)
	{
		pev->spawnflags &= ~SF_TRAIN_WAIT_RETRIGGER;
		Next();
	}
	else
	{
		pev->spawnflags |= SF_TRAIN_WAIT_RETRIGGER;

		if (pev->enemy)
			pev->target = pev->enemy->v.targetname;

		pev->nextthink = 0;
		pev->velocity = g_vecZero;

		if (pev->noiseStopMoving)
			EMIT_SOUND(ENT(pev), CHAN_VOICE, (char *)STRING(pev->noiseStopMoving), m_volume, ATTN_NORM);
	}
}

void CFuncTrain::Wait(void)
{
	if (m_pevCurrentTarget->message)
	{
		FireTargets(STRING(m_pevCurrentTarget->message), this, this, USE_TOGGLE, 0);

		if (FBitSet(m_pevCurrentTarget->spawnflags, SF_CORNER_FIREONCE))
			m_pevCurrentTarget->message = 0;
	}

	if (FBitSet(m_pevCurrentTarget->spawnflags , SF_TRAIN_WAIT_RETRIGGER) || (pev->spawnflags & SF_TRAIN_WAIT_RETRIGGER))
	{
		pev->spawnflags |= SF_TRAIN_WAIT_RETRIGGER;

		if (pev->noiseMovement)
			STOP_SOUND(edict(), CHAN_STATIC, (char *)STRING(pev->noiseMovement));

		if (pev->noiseStopMoving)
			EMIT_SOUND(ENT(pev), CHAN_VOICE, (char *)STRING(pev->noiseStopMoving), m_volume, ATTN_NORM);

		pev->nextthink = 0;
		return;
	}

	if (m_flWait != 0)
	{
		pev->nextthink = pev->ltime + m_flWait;

		if (pev->noiseMovement)
			STOP_SOUND(edict(), CHAN_STATIC, (char *)STRING(pev->noiseMovement));

		if (pev->noiseStopMoving)
			EMIT_SOUND(ENT(pev), CHAN_VOICE, (char *)STRING(pev->noiseStopMoving), m_volume, ATTN_NORM);

		SetThink(&CFuncTrain::Next);
	}
	else
		Next();
}

void CFuncTrain::Next(void)
{
	CBaseEntity *pTarg = GetNextTarget();

	if (!pTarg)
	{
		if (pev->noiseMovement)
			STOP_SOUND(edict(), CHAN_STATIC, (char *)STRING(pev->noiseMovement));

		if (pev->noiseStopMoving)
			EMIT_SOUND(ENT(pev), CHAN_VOICE, (char *)STRING(pev->noiseStopMoving), m_volume, ATTN_NORM);

		return;
	}

	pev->message = pev->target;
	pev->target = pTarg->pev->target;
	m_flWait = pTarg->GetDelay();

	if (m_pevCurrentTarget && m_pevCurrentTarget->speed != 0)
	{
		pev->speed = m_pevCurrentTarget->speed;
		ALERT(at_aiconsole, "Train %s speed to %4.2f\n", STRING(pev->targetname), pev->speed);
	}

	m_pevCurrentTarget = pTarg->pev;
	pev->enemy = pTarg->edict();

	if (FBitSet(m_pevCurrentTarget->spawnflags, SF_CORNER_TELEPORT))
	{
		SetBits(pev->effects, EF_NOINTERP);
		UTIL_SetOrigin(pev, pTarg->pev->origin - (pev->mins + pev->maxs) * 0.5);
		Wait();
	}
	else
	{
		if (pev->noiseMovement)
			STOP_SOUND(edict(), CHAN_STATIC, (char *)STRING(pev->noiseMovement));

		if (pev->noiseMovement)
			EMIT_SOUND(ENT(pev), CHAN_STATIC, (char *)STRING(pev->noiseMovement), m_volume, ATTN_NORM);

		ClearBits(pev->effects, EF_NOINTERP);
		SetMoveDone(&CFuncTrain::Wait);
		LinearMove(pTarg->pev->origin - (pev->mins + pev->maxs) * 0.5, pev->speed);
	}
}

void CFuncTrain::Activate(void)
{
	if (!m_activated)
	{
		m_activated = TRUE;
		entvars_t *pevTarg = VARS(FIND_ENTITY_BY_TARGETNAME(NULL, STRING(pev->target)));
		pev->target = pevTarg->target;
		m_pevCurrentTarget = pevTarg;

		UTIL_SetOrigin(pev, pevTarg->origin - (pev->mins + pev->maxs) * 0.5);

		if (FStringNull(pev->targetname))
		{
			pev->nextthink = pev->ltime + 0.1;
			SetThink(&CFuncTrain::Next);
		}
		else
			pev->spawnflags |= SF_TRAIN_WAIT_RETRIGGER;
	}
}

void CFuncTrain::Spawn(void)
{
	Precache();

	if (pev->speed == 0)
		pev->speed = 100;

	if (FStringNull(pev->target))
		ALERT(at_console, "FuncTrain with no target");

	m_pevSaveTarget = m_pevCurrentTarget;
	m_vecSaveOrigin = pev->origin;

	if (pev->dmg < 0) //hz fixed
		pev->dmg = 0;

	pev->movetype = MOVETYPE_PUSH;

	if (FBitSet(pev->spawnflags, SF_TRACKTRAIN_PASSABLE))
		pev->solid = SOLID_NOT;
	else
		pev->solid = SOLID_BSP;

	SET_MODEL(ENT(pev), STRING(pev->model));
	UTIL_SetSize(pev, pev->mins, pev->maxs);
	UTIL_SetOrigin(pev, pev->origin);

	m_activated = FALSE;

	if (m_volume == 0)
		m_volume = 0.85;
}

void CFuncTrain::Restart(void)
{
	if (pev->speed == 0)
		pev->speed = 100;

	if (pev->dmg == 0)
		pev->dmg = 2;

	pev->movetype = MOVETYPE_PUSH;
	m_pevCurrentTarget = m_pevSaveTarget;
	UTIL_SetOrigin(pev, m_vecSaveOrigin);

	m_activated = FALSE;

	if (m_volume == 0)
		m_volume = 0.85;
}

void CFuncTrain::Precache(void)
{
	CBasePlatTrain::Precache();
}

void CFuncTrain::OverrideReset(void)
{
	if (pev->velocity != g_vecZero && pev->nextthink != 0)
	{
		pev->target = pev->message;
		CBaseEntity *pTarg = GetNextTarget();

		if (!pTarg)
		{
			pev->nextthink = 0;
			pev->velocity = g_vecZero;
		}
		else
		{
			SetThink(&CFuncTrain::Next);
			pev->nextthink = pev->ltime + 0.1;
		}
	}
}

LINK_ENTITY_TO_CLASS(func_tracktrain, CFuncTrackTrain);

void CFuncTrackTrain::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "wheels"))
	{
		m_length = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "height"))
	{
		m_height = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "startspeed"))
	{
		m_startSpeed = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "sounds"))
	{
		m_sounds = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "volume"))
	{
		m_flVolume = (float)(atoi(pkvd->szValue));
		m_flVolume *= 0.1;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "bank"))
	{
		m_flBank = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

void CFuncTrackTrain::NextThink(float thinkTime, BOOL alwaysThink)
{
	if (alwaysThink)
		pev->flags |= FL_ALWAYSTHINK;
	else
		pev->flags &= ~FL_ALWAYSTHINK;

	pev->nextthink = thinkTime;
}

void CFuncTrackTrain::Blocked(CBaseEntity *pOther)
{
	entvars_t *pevOther = pOther->pev;

	if (FBitSet(pevOther->flags, FL_ONGROUND) && VARS(pevOther->groundentity) == pev)
	{
		float deltaSpeed = fabs(pev->speed);

		if (deltaSpeed > 50)
			deltaSpeed = 50;

		if (!pevOther->velocity.z)
			pevOther->velocity.z += deltaSpeed;

		return;
	}
	else
		pevOther->velocity = (pevOther->origin - pev->origin).Normalize() * pev->dmg;

	ALERT(at_aiconsole, "TRAIN(%s): Blocked by %s (dmg:%.2f)\n", STRING(pev->targetname), STRING(pOther->pev->classname), pev->dmg);

	if (pev->dmg <= 0)
		return;

	pOther->TakeDamage( pev, pev, pev->dmg, DMG_CRUSH, 0 );	
}

void CFuncTrackTrain::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (useType != USE_SET)
	{
		if (!ShouldToggle(useType, (pev->speed != 0)))
			return;

		if (pev->speed == 0)
		{
			pev->speed = m_speed * m_dir;
			Next();
		}
		else
		{
			pev->speed = 0;
			pev->velocity = g_vecZero;
			pev->avelocity = g_vecZero;
			StopSound();
			SetThink(NULL);
		}
	}
	else
	{
		float delta = value;
		delta = ((int)(pev->speed * 4) / (int)m_speed) * 0.25 + 0.25 * delta;

		if (delta > 1)
			delta = 1;
		else if (delta < -1)
			delta = -1;

		if (pev->spawnflags & SF_TRACKTRAIN_FORWARDONLY)
		{
			if (delta < 0)
				delta = 0;
		}

		pev->speed = m_speed * delta;
		Next();
		ALERT(at_aiconsole, "TRAIN(%s), speed to %.2f\n", STRING(pev->targetname), pev->speed);
	}
}

static float Fix(float angle)
{
	while (angle < 0)
		angle += 360;
	while (angle > 360)
		angle -= 360;

	return angle;
}

static void FixupAngles(Vector &v)
{
	v.x = Fix(v.x);
	v.y = Fix(v.y);
	v.z = Fix(v.z);
}

#define TRAIN_STARTPITCH 60
#define TRAIN_MAXPITCH 200
#define TRAIN_MAXSPEED 1000

void CFuncTrackTrain::StopSound(void)
{
	if (m_soundPlaying && pev->noise)
	{
		unsigned short us_sound = ((unsigned short)m_sounds & 0x0007) << 12;
		unsigned short us_encode = us_sound;

		PLAYBACK_EVENT_FULL(FEV_RELIABLE | FEV_UPDATE, edict(), m_usAdjustPitch, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, us_encode, 0, 1, 0);
	}

	m_soundPlaying = 0;
}

void CFuncTrackTrain::UpdateSound(void)
{
	if (!pev->noise)
		return;

	float flpitch = TRAIN_STARTPITCH + (abs((int)pev->speed) * (TRAIN_MAXPITCH - TRAIN_STARTPITCH) / TRAIN_MAXSPEED);

	if (!m_soundPlaying)
	{
		EMIT_SOUND_DYN(ENT(pev), CHAN_ITEM, "plats/ttrain_start1.wav", m_flVolume, ATTN_NORM, 0, 100);
		EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, (char *)STRING(pev->noise), m_flVolume, ATTN_NORM, 0, (int)flpitch);
		m_soundPlaying = 1;
	}
	else
	{
		unsigned short us_sound = ((unsigned short)(m_sounds) & 0x0007) << 12;
		unsigned short us_pitch = ((unsigned short)(flpitch / 10.0) & 0x003F) << 6;
		unsigned short us_volume = ((unsigned short)(m_flVolume * 40) & 0x003F);
		unsigned short us_encode = us_sound | us_pitch | us_volume;

		PLAYBACK_EVENT_FULL(FEV_RELIABLE | FEV_UPDATE, edict(), m_usAdjustPitch, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, us_encode, 0, 0, 0);
	}
}

void CFuncTrackTrain::Next(void)
{
	float time = 0.5;

	if (!pev->speed)
	{
		ALERT(at_aiconsole, "TRAIN(%s): Speed is 0\n", STRING(pev->targetname));
		StopSound();
		return;
	}

	if (!m_ppath)
	{
		ALERT(at_aiconsole, "TRAIN(%s): Lost path\n", STRING(pev->targetname));
		StopSound();
		return;
	}

	UpdateSound();

	Vector nextPos = pev->origin;
	nextPos.z -= m_height;
	CPathTrack *pnext = m_ppath->LookAhead(&nextPos, pev->speed * 0.1, 1);
	nextPos.z += m_height;

	pev->velocity = (nextPos - pev->origin) * 10;
	Vector nextFront = pev->origin;
	nextFront.z -= m_height;

	if (m_length > 0)
		m_ppath->LookAhead(&nextFront, m_length, 0);
	else
		m_ppath->LookAhead(&nextFront, 100, 0);

	nextFront.z += m_height;

	Vector delta = nextFront - pev->origin;
	Vector angles = UTIL_VecToAngles(delta);
	angles.y += 180;

	FixupAngles(angles);
	FixupAngles(pev->angles);

	if (!pnext || (delta.x == 0 && delta.y == 0))
		angles = pev->angles;

	float vy, vx;

	if (!(pev->spawnflags & SF_TRACKTRAIN_NOPITCH))
		vx = UTIL_AngleDistance(angles.x, pev->angles.x);
	else
		vx = 0;

	vy = UTIL_AngleDistance(angles.y, pev->angles.y);
	pev->avelocity.y = vy * 10;
	pev->avelocity.x = vx * 10;

	if (m_flBank != 0)
	{
		if (pev->avelocity.y < -5)
			pev->avelocity.z = UTIL_AngleDistance(UTIL_ApproachAngle(-m_flBank, pev->angles.z, m_flBank * 2), pev->angles.z);
		else if (pev->avelocity.y > 5)
			pev->avelocity.z = UTIL_AngleDistance(UTIL_ApproachAngle(m_flBank, pev->angles.z, m_flBank * 2), pev->angles.z);
		else
			pev->avelocity.z = UTIL_AngleDistance(UTIL_ApproachAngle(0, pev->angles.z, m_flBank * 4), pev->angles.z) * 4;
	}

	if (pnext)
	{
		if (pnext != m_ppath)
		{
			CPathTrack *pFire = (pev->speed >= 0) ? pnext : m_ppath;
			m_ppath = pnext;

			if (pFire->pev->message)
			{
				FireTargets(STRING(pFire->pev->message), this, this, USE_TOGGLE, 0);

				if (FBitSet(pFire->pev->spawnflags, SF_PATH_FIREONCE))
					pFire->pev->message = 0;
			}

			if (pFire->pev->spawnflags & SF_PATH_DISABLE_TRAIN)
				pev->spawnflags |= SF_TRACKTRAIN_NOCONTROL;

			if (pev->spawnflags & SF_TRACKTRAIN_NOCONTROL)
			{
				if (pFire->pev->speed != 0)
				{
					pev->speed = pFire->pev->speed;
					ALERT(at_aiconsole, "TrackTrain %s speed to %4.2f\n", STRING(pev->targetname), pev->speed);
				}
			}
		}

		SetThink(&CFuncTrackTrain::Next);
		NextThink(pev->ltime + time, TRUE);
	}
	else
	{
		StopSound();
		pev->velocity = (nextPos - pev->origin);
		pev->avelocity = g_vecZero;

		float distance = pev->velocity.Length();
		m_oldSpeed = pev->speed;
		pev->speed = 0;

		if (distance > 0)
		{
			time = distance / m_oldSpeed;
			pev->velocity = pev->velocity * (m_oldSpeed / distance);
			SetThink(&CFuncTrackTrain::DeadEnd);
			NextThink(pev->ltime + time, FALSE);
		}
		else
			DeadEnd();
	}
}

void CFuncTrackTrain::DeadEnd(void)
{
	CPathTrack *pTrack = m_ppath;
	ALERT(at_aiconsole, "TRAIN(%s): Dead end ", STRING(pev->targetname));

	if (pTrack)
	{
		CPathTrack *pNext;

		if (m_oldSpeed < 0)
		{
			do
			{
				pNext = pTrack->ValidPath(pTrack->GetPrevious(), TRUE);

				if (pNext)
					pTrack = pNext;
			}
			while (pNext);
		}
		else
		{
			do
			{
				pNext = pTrack->ValidPath(pTrack->GetNext(), TRUE);

				if (pNext)
					pTrack = pNext;
			}
			while (pNext);
		}
	}

	pev->velocity = g_vecZero;
	pev->avelocity = g_vecZero;

	if (pTrack)
	{
		ALERT(at_aiconsole, "at %s\n", STRING(pTrack->pev->targetname));

		if (pTrack->pev->netname)
			FireTargets(STRING(pTrack->pev->netname), this, this, USE_TOGGLE, 0);
	}
	else
		ALERT(at_aiconsole, "\n");
}

void CFuncTrackTrain::SetControls(entvars_t *pevControls)
{
	Vector offset = pevControls->origin - pev->oldorigin;
	m_controlMins = pevControls->mins + offset;
	m_controlMaxs = pevControls->maxs + offset;
}

BOOL CFuncTrackTrain::OnControls(entvars_t *pevTest)
{
	Vector offset = pevTest->origin - pev->origin;

	if (pev->spawnflags & SF_TRACKTRAIN_NOCONTROL)
		return FALSE;

	UTIL_MakeVectors(pev->angles);

	Vector local;
	local.x = DotProduct(offset, gpGlobals->v_forward);
	local.y = -DotProduct(offset, gpGlobals->v_right);
	local.z = DotProduct(offset, gpGlobals->v_up);

	if (local.x >= m_controlMins.x && local.y >= m_controlMins.y && local.z >= m_controlMins.z && local.x <= m_controlMaxs.x && local.y <= m_controlMaxs.y && local.z <= m_controlMaxs.z)
		return TRUE;

	return FALSE;
}

void CFuncTrackTrain::Find(void)
{
	m_ppath = CPathTrack::Instance(FIND_ENTITY_BY_TARGETNAME(NULL, STRING(pev->target)));

	if (!m_ppath)
		return;

	entvars_t *pevTarget = m_ppath->pev;

	if (!FClassnameIs(pevTarget, "path_track"))
	{
		ALERT(at_error, "func_track_train must be on a path of path_track\n");
		m_ppath = NULL;
		return;
	}

	Vector nextPos = pevTarget->origin;
	nextPos.z += m_height;

	Vector look = nextPos;
	look.z -= m_height;
	m_ppath->LookAhead(&look, m_length, 0);
	look.z += m_height;

	pev->angles = UTIL_VecToAngles(look - nextPos);
	pev->angles.y += 180;

	if (pev->spawnflags & SF_TRACKTRAIN_NOPITCH)
		pev->angles.x = 0;

	UTIL_SetOrigin(pev, nextPos);
	NextThink(pev->ltime + 0.1, FALSE);
	SetThink(&CFuncTrackTrain::Next);
	pev->speed = m_startSpeed;
	UpdateSound();
}

void CFuncTrackTrain::NearestPath(void)
{
	CBaseEntity *pTrack = NULL;
	CBaseEntity *pNearest = NULL;
	float closest = 1024;

	while ((pTrack = UTIL_FindEntityInSphere(pTrack, pev->origin, 1024)) != NULL)
	{
		if (!(pTrack->pev->flags & (FL_CLIENT | FL_MONSTER)) && FClassnameIs(pTrack->pev, "path_track"))
		{
			float dist = (pev->origin - pTrack->pev->origin).Length();

			if (dist < closest)
			{
				closest = dist;
				pNearest = pTrack;
			}
		}
	}

	if (!pNearest)
	{
		ALERT(at_console, "Can't find a nearby track !!!\n");
		SetThink(NULL);
		return;
	}

	ALERT(at_aiconsole, "TRAIN: %s, Nearest track is %s\n", STRING(pev->targetname), STRING(pNearest->pev->targetname));
	pTrack = ((CPathTrack *)pNearest)->GetNext();

	if (pTrack)
	{
		if ((pev->origin - pTrack->pev->origin).Length() < (pev->origin - pNearest->pev->origin).Length())
			pNearest = pTrack;
	}

	m_ppath = (CPathTrack *)pNearest;

	if (pev->speed != 0)
	{
		NextThink(pev->ltime + 0.1, FALSE);
		SetThink(&CFuncTrackTrain::Next);
	}
}

void CFuncTrackTrain::OverrideReset(void)
{
	NextThink(pev->ltime + 0.1, FALSE);
	SetThink(&CFuncTrackTrain::NearestPath);
}

CFuncTrackTrain *CFuncTrackTrain::Instance(edict_t *pent)
{
	if (FClassnameIs(pent, "func_tracktrain"))
		return (CFuncTrackTrain *)GET_PRIVATE(pent);

	return NULL;
}

void CFuncTrackTrain::Spawn(void)
{
	if (pev->speed == 0)
		m_speed = 165;
	else
		m_speed = pev->speed;

	ALERT(at_console, "M_speed = %f\n", m_speed);

	pev->speed = 0;
	pev->velocity = g_vecZero;
	pev->avelocity = g_vecZero;
	pev->impulse = (int)m_speed;
	m_dir = 1;

	if (FStringNull(pev->target))
		ALERT(at_console, "FuncTrain with no target");

	if (pev->spawnflags & SF_TRACKTRAIN_PASSABLE)
		pev->solid = SOLID_NOT;
	else
		pev->solid = SOLID_BSP;

	pev->movetype = MOVETYPE_PUSH;

	SET_MODEL(ENT(pev), STRING(pev->model));
	UTIL_SetSize(pev, pev->mins, pev->maxs);
	UTIL_SetOrigin(pev, pev->origin);

	pev->oldorigin = pev->origin;

	m_controlMins = pev->mins;
	m_controlMaxs = pev->maxs;
	m_controlMaxs.z += 72;

	NextThink(pev->ltime + 0.1, FALSE);
	SetThink(&CFuncTrackTrain::Find);
	Precache();
}

void CFuncTrackTrain::Restart(void)
{
	ALERT(at_console, "M_speed = %f\n", m_speed);

	pev->speed = 0;
	pev->velocity = g_vecZero;
	pev->avelocity = g_vecZero;
	pev->impulse = (int)m_speed;
	m_dir = 1;

	if (FStringNull(pev->target))
		ALERT(at_console, "FuncTrain with no target");

	UTIL_SetOrigin(pev, pev->oldorigin);
	NextThink(pev->ltime + 0.1, FALSE);
	SetThink(&CFuncTrackTrain::Find);
}

void CFuncTrackTrain::Precache(void)
{
	if (m_flVolume == 0)
		m_flVolume = 1;

	switch (m_sounds)
	{
		default: pev->noise = 0; break;
		case 1: PRECACHE_SOUND("plats/ttrain1.wav"); pev->noise = MAKE_STRING("plats/ttrain1.wav"); break;
		case 2: PRECACHE_SOUND("plats/ttrain2.wav"); pev->noise = MAKE_STRING("plats/ttrain2.wav"); break;
		case 3: PRECACHE_SOUND("plats/ttrain3.wav"); pev->noise = MAKE_STRING("plats/ttrain3.wav"); break;
		case 4: PRECACHE_SOUND("plats/ttrain4.wav"); pev->noise = MAKE_STRING("plats/ttrain4.wav"); break;
		case 5: PRECACHE_SOUND("plats/ttrain6.wav"); pev->noise = MAKE_STRING("plats/ttrain6.wav"); break;
		case 6: PRECACHE_SOUND("plats/ttrain7.wav"); pev->noise = MAKE_STRING("plats/ttrain7.wav"); break;
	}

	PRECACHE_SOUND("plats/ttrain_brake1.wav");
	PRECACHE_SOUND("plats/ttrain_start1.wav");
	m_usAdjustPitch = PRECACHE_EVENT(1, "events/train.sc");
}

class CFuncTrainControls : public CBaseEntity
{
public:
	int ObjectCaps(void) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	void Spawn(void);
	void EXPORT Find(void);
};

LINK_ENTITY_TO_CLASS(func_traincontrols, CFuncTrainControls);

void CFuncTrainControls::Find(void)
{
	edict_t *pTarget = NULL;
	do pTarget = FIND_ENTITY_BY_TARGETNAME(pTarget, STRING(pev->target));
	while (!FNullEnt(pTarget) && !FClassnameIs(pTarget, "func_tracktrain"));

	if (FNullEnt(pTarget))
	{
		ALERT(at_console, "No train %s\n", STRING(pev->target));
		return;
	}

	CFuncTrackTrain *ptrain = CFuncTrackTrain::Instance(pTarget);
	ptrain->SetControls(pev);
	UTIL_Remove(this);
}

void CFuncTrainControls::Spawn(void)
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	SET_MODEL(ENT(pev), STRING(pev->model));

	UTIL_SetSize(pev, pev->mins, pev->maxs);
	UTIL_SetOrigin(pev, pev->origin);

	SetThink(&CFuncTrainControls::Find);
	pev->nextthink = gpGlobals->time;
}

#define SF_TRACK_ACTIVATETRAIN 0x00000001
#define SF_TRACK_RELINK 0x00000002
#define SF_TRACK_ROTMOVE 0x00000004
#define SF_TRACK_STARTBOTTOM 0x00000008
#define SF_TRACK_DONT_MOVE 0x00000010

typedef enum { TRAIN_SAFE, TRAIN_BLOCKING, TRAIN_FOLLOWING } TRAIN_CODE;

class CFuncTrackChange : public CFuncPlatRot
{
public:
	virtual void Spawn(void);
	virtual void Precache(void);
	virtual void EXPORT GoUp(void);
	virtual void EXPORT GoDown(void);
	virtual void KeyValue(KeyValueData *pkvd);
	virtual void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	virtual void HitBottom(void);
	virtual void HitTop(void);
	virtual void UpdateAutoTargets(int toggleState);
	virtual BOOL IsTogglePlat(void) { return TRUE; }
	virtual void Touch(CBaseEntity *pOther);
	virtual void OverrideReset(void);

public:
	TRAIN_CODE EvaluateTrain(CPathTrack *pcurrent);
	void UpdateTrain(Vector &dest);
	void EXPORT Find(void);
	void DisableUse(void) { m_use = 0; }
	void EnableUse(void) { m_use = 1; }
	int UseEnabled(void) { return m_use; }

public:
	CPathTrack *m_trackTop;
	CPathTrack *m_trackBottom;
	CFuncTrackTrain *m_train;
	int m_trackTopName;
	int m_trackBottomName;
	int m_trainName;
	TRAIN_CODE m_code;
	int m_targetState;
	int m_use;
};

LINK_ENTITY_TO_CLASS(func_trackchange, CFuncTrackChange);

void CFuncTrackChange::Spawn(void)
{
	Setup();

	if (FBitSet(pev->spawnflags, SF_TRACK_DONT_MOVE))
		m_vecPosition2.z = pev->origin.z;

	SetupRotation();

	if (FBitSet(pev->spawnflags, SF_TRACK_STARTBOTTOM))
	{
		UTIL_SetOrigin(pev, m_vecPosition2);
		m_toggle_state = TS_AT_BOTTOM;
		pev->angles = m_start;
		m_targetState = TS_AT_TOP;
	}
	else
	{
		UTIL_SetOrigin(pev, m_vecPosition1);
		m_toggle_state = TS_AT_TOP;
		pev->angles = m_end;
		m_targetState = TS_AT_BOTTOM;
	}

	EnableUse();
	pev->nextthink = pev->ltime + 2;
	SetThink(&CFuncTrackChange::Find);
	Precache();
}

void CFuncTrackChange::Precache(void)
{
	PRECACHE_SOUND("buttons/button11.wav");
	CFuncPlatRot::Precache();
}

void CFuncTrackChange::Touch(CBaseEntity *pOther)
{
}

void CFuncTrackChange::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "train"))
	{
		m_trainName = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "toptrack"))
	{
		m_trackTopName = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "bottomtrack"))
	{
		m_trackBottomName = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CFuncPlatRot::KeyValue(pkvd);
}

void CFuncTrackChange::OverrideReset(void)
{
	pev->nextthink = pev->ltime + 1;
	SetThink(&CFuncTrackChange::Find);
}

void CFuncTrackChange::Find(void)
{
	edict_t *target = FIND_ENTITY_BY_TARGETNAME(NULL, STRING(m_trackTopName));

	if (!FNullEnt(target))
	{
		m_trackTop = CPathTrack::Instance(target);
		target = FIND_ENTITY_BY_TARGETNAME(NULL, STRING(m_trackBottomName));

		if (!FNullEnt(target))
		{
			m_trackBottom = CPathTrack::Instance(target);
			target = FIND_ENTITY_BY_TARGETNAME(NULL, STRING(m_trainName));

			if (!FNullEnt(target))
			{
				m_train = CFuncTrackTrain::Instance(FIND_ENTITY_BY_TARGETNAME(NULL, STRING(m_trainName)));

				if (!m_train)
				{
					ALERT(at_error, "Can't find train for track change! %s\n", STRING(m_trainName));
					return;
				}

				Vector center = (pev->absmin + pev->absmax) * 0.5;
				m_trackBottom = m_trackBottom->Nearest(center);
				m_trackTop = m_trackTop->Nearest(center);
				UpdateAutoTargets(m_toggle_state);
				SetThink(NULL);
				return;
			}
			else
			{
				ALERT(at_error, "Can't find train for track change! %s\n", STRING(m_trainName));
				target = FIND_ENTITY_BY_TARGETNAME(NULL, STRING(m_trainName));
			}
		}
		else
			ALERT(at_error, "Can't find bottom track for track change! %s\n", STRING(m_trackBottomName));
	}
	else
		ALERT(at_error, "Can't find top track for track change! %s\n", STRING(m_trackTopName));
}

TRAIN_CODE CFuncTrackChange::EvaluateTrain(CPathTrack *pcurrent)
{
	if (!pcurrent || !m_train)
		return TRAIN_SAFE;

	if (m_train->m_ppath == pcurrent || (pcurrent->m_pprevious && m_train->m_ppath == pcurrent->m_pprevious) || (pcurrent->m_pnext && m_train->m_ppath == pcurrent->m_pnext))
	{
		if (m_train->pev->speed != 0)
			return TRAIN_BLOCKING;

		Vector dist = pev->origin - m_train->pev->origin;
		float length = dist.Length2D();

		if (length < m_train->m_length)
			return TRAIN_FOLLOWING;
		else if (length > (150 + m_train->m_length))
			return TRAIN_SAFE;

		return TRAIN_BLOCKING;
	}

	return TRAIN_SAFE;
}

void CFuncTrackChange::UpdateTrain(Vector &dest)
{
	float time = (pev->nextthink - pev->ltime);
	m_train->pev->velocity = pev->velocity;
	m_train->pev->avelocity = pev->avelocity;
	m_train->NextThink(m_train->pev->ltime + time, FALSE);

	if (time <= 0)
		return;

	Vector offset = m_train->pev->origin - pev->origin;
	Vector delta = dest - pev->angles;
	UTIL_MakeInvVectors(delta, gpGlobals);

	Vector local;
	local.x = DotProduct(offset, gpGlobals->v_forward);
	local.y = DotProduct(offset, gpGlobals->v_right);
	local.z = DotProduct(offset, gpGlobals->v_up);
	local = local - offset;
	m_train->pev->velocity = pev->velocity + (local * (1.0 / time));
}

void CFuncTrackChange::GoDown(void)
{
	if (m_code == TRAIN_BLOCKING)
		return;

	UpdateAutoTargets(TS_GOING_DOWN);

	if (FBitSet(pev->spawnflags, SF_TRACK_DONT_MOVE))
	{
		SetMoveDone(&CFuncPlat::CallHitBottom);
		m_toggle_state = TS_GOING_DOWN;
		AngularMove(m_start, pev->speed);
	}
	else
	{
		CFuncPlat::GoDown();
		SetMoveDone(&CFuncPlat::CallHitBottom);
		RotMove(m_start, pev->nextthink - pev->ltime);
	}

	if (m_code == TRAIN_FOLLOWING)
	{
		UpdateTrain(m_start);
		m_train->m_ppath = NULL;
	}
}

void CFuncTrackChange::GoUp(void)
{
	if (m_code == TRAIN_BLOCKING)
		return;

	UpdateAutoTargets(TS_GOING_UP);

	if (FBitSet(pev->spawnflags, SF_TRACK_DONT_MOVE))
	{
		m_toggle_state = TS_GOING_UP;
		SetMoveDone(&CFuncPlat::CallHitTop);
		AngularMove(m_end, pev->speed);
	}
	else
	{
		CFuncPlat::GoUp();
		SetMoveDone(&CFuncPlat::CallHitTop);
		RotMove(m_end, pev->nextthink - pev->ltime);
	}

	if (m_code == TRAIN_FOLLOWING)
	{
		UpdateTrain(m_end);
		m_train->m_ppath = NULL;
	}
}

void CFuncTrackChange::UpdateAutoTargets(int toggleState)
{
	if (!m_trackTop || !m_trackBottom)
		return;

	if (toggleState == TS_AT_TOP)
		ClearBits(m_trackTop->pev->spawnflags, SF_PATH_DISABLED);
	else
		SetBits(m_trackTop->pev->spawnflags, SF_PATH_DISABLED);

	if (toggleState == TS_AT_BOTTOM)
		ClearBits(m_trackBottom->pev->spawnflags, SF_PATH_DISABLED);
	else
		SetBits(m_trackBottom->pev->spawnflags, SF_PATH_DISABLED);
}

void CFuncTrackChange::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (m_toggle_state != TS_AT_TOP && m_toggle_state != TS_AT_BOTTOM)
		return;

	if (m_toggle_state == TS_AT_TOP)
		m_code = EvaluateTrain(m_trackTop);
	else if (m_toggle_state == TS_AT_BOTTOM)
		m_code = EvaluateTrain(m_trackBottom);
	else
		m_code = TRAIN_BLOCKING;

	if (m_code == TRAIN_BLOCKING)
	{
		EMIT_SOUND(ENT(pev), CHAN_VOICE, "buttons/button11.wav", VOL_NORM, ATTN_NORM);
		return;
	}

	DisableUse();

	if (m_toggle_state == TS_AT_TOP)
		GoDown();
	else
		GoUp();
}

void CFuncTrackChange::HitBottom(void)
{
	CFuncPlatRot::HitBottom();

	if (m_code == TRAIN_FOLLOWING)
		m_train->SetTrack(m_trackBottom);

	SetThink(NULL);
	pev->nextthink = -1;
	UpdateAutoTargets(m_toggle_state);
	EnableUse();
}

void CFuncTrackChange::HitTop(void)
{
	CFuncPlatRot::HitTop();

	if (m_code == TRAIN_FOLLOWING)
		m_train->SetTrack(m_trackTop);

	SetThink(NULL);
	pev->nextthink = -1;
	UpdateAutoTargets(m_toggle_state);
	EnableUse();
}

class CFuncTrackAuto : public CFuncTrackChange
{
public:
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void UpdateAutoTargets(int toggleState);
};

LINK_ENTITY_TO_CLASS(func_trackautochange, CFuncTrackAuto);

void CFuncTrackAuto::UpdateAutoTargets(int toggleState)
{
	CPathTrack *pTarget, *pNextTarget;

	if (!m_trackTop || !m_trackBottom)
		return;

	if (m_targetState == TS_AT_TOP)
	{
		pTarget = m_trackTop->GetNext();
		pNextTarget = m_trackBottom->GetNext();
	}
	else
	{
		pTarget = m_trackBottom->GetNext();
		pNextTarget = m_trackTop->GetNext();
	}

	if (pTarget)
	{
		ClearBits(pTarget->pev->spawnflags, SF_PATH_DISABLED);

		if (m_code == TRAIN_FOLLOWING && m_train && m_train->pev->speed == 0)
			m_train->Use(this, this, USE_ON, 0);
	}

	if (pNextTarget)
		SetBits(pNextTarget->pev->spawnflags, SF_PATH_DISABLED);
}

void CFuncTrackAuto::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	CPathTrack *pTarget;

	if (!UseEnabled())
		return;

	if (m_toggle_state == TS_AT_TOP)
		pTarget = m_trackTop;
	else if (m_toggle_state == TS_AT_BOTTOM)
		pTarget = m_trackBottom;
	else
		pTarget = NULL;

	if (FClassnameIs(pActivator->pev, "func_tracktrain"))
	{
		m_code = EvaluateTrain(pTarget);

		if (m_code == TRAIN_FOLLOWING && m_toggle_state != m_targetState)
		{
			DisableUse();

			if (m_toggle_state == TS_AT_TOP)
				GoDown();
			else
				GoUp();
		}
	}
	else
	{
		if (pTarget)
			pTarget = pTarget->GetNext();

		if (pTarget && m_train->m_ppath != pTarget && ShouldToggle(useType, m_targetState))
		{
			if (m_targetState == TS_AT_TOP)
				m_targetState = TS_AT_BOTTOM;
			else
				m_targetState = TS_AT_TOP;
		}

		UpdateAutoTargets(m_targetState);
	}
}

#define FGUNTARGET_START_ON 0x0001

class CGunTarget : public CBaseMonster
{
public:
	void Spawn(void);
	void Activate(void);
	void EXPORT Next(void);
	void EXPORT Start(void);
	void EXPORT Wait(void);
	void Stop(void);

public:
	int BloodColor(void) { return DONT_BLEED; }
	int Classify(void) { return CLASS_MOVEABLE; }
	int TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType, int iCrit);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	Vector BodyTarget(const Vector &posSrc) { return pev->origin; }
	int ObjectCaps(void) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

private:
	BOOL m_on;
};

LINK_ENTITY_TO_CLASS(func_guntarget, CGunTarget);

void CGunTarget::Spawn(void)
{
	pev->solid = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;

	UTIL_SetOrigin(pev, pev->origin);
	SET_MODEL(ENT(pev), STRING(pev->model));

	if (pev->speed == 0)
		pev->speed = 100;

	pev->takedamage = DAMAGE_NO;
	pev->flags |= FL_MONSTER;
	m_on = FALSE;
	pev->max_health = pev->health;

	if (pev->spawnflags & FGUNTARGET_START_ON)
	{
		SetThink(&CGunTarget::Start);
		pev->nextthink = pev->ltime + 0.3;
	}
}

void CGunTarget::Activate(void)
{
	CBaseEntity *pTarg = GetNextTarget();

	if (pTarg)
	{
		m_hTargetEnt = pTarg;
		UTIL_SetOrigin(pev, pTarg->pev->origin - (pev->mins + pev->maxs) * 0.5);
	}
}

void CGunTarget::Start(void)
{
	Use(this, this, USE_ON, 0);
}

void CGunTarget::Next(void)
{
	SetThink(NULL);
	m_hTargetEnt = GetNextTarget();
	CBaseEntity *pTarget = m_hTargetEnt;

	if (!pTarget)
	{
		Stop();
		return;
	}

	SetMoveDone(&CGunTarget::Wait);
	LinearMove(pTarget->pev->origin - (pev->mins + pev->maxs) * 0.5, pev->speed);
}

void CGunTarget::Wait(void)
{
	CBaseEntity *pTarget = m_hTargetEnt;

	if (!pTarget)
	{
		Stop();
		return;
	}

	if (pTarget->pev->message)
	{
		FireTargets(STRING(pTarget->pev->message), this, this, USE_TOGGLE, 0);

		if (FBitSet(pTarget->pev->spawnflags, SF_CORNER_FIREONCE))
			pTarget->pev->message = 0;
	}

	m_flWait = pTarget->GetDelay();
	pev->target = pTarget->pev->target;
	SetThink(&CGunTarget::Next);

	if (m_flWait != 0)
		pev->nextthink = pev->ltime + m_flWait;
	else
		Next();
}

void CGunTarget::Stop(void)
{
	pev->velocity = g_vecZero;
	pev->nextthink = 0;
	pev->takedamage = DAMAGE_NO;
}

int CGunTarget::TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType, int iCrit)
{
	if (pev->health > 0)
	{
		pev->health -= flDamage;

		if (pev->health <= 0)
		{
			pev->health = 0;
			Stop();

			if (pev->message)
				FireTargets(STRING(pev->message), this, this, USE_TOGGLE, 0);
		}
	}

	return 0;
}

void CGunTarget::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (!ShouldToggle(useType, m_on))
		return;

	if (m_on)
	{
		Stop();
	}
	else
	{
		pev->takedamage = DAMAGE_AIM;
		m_hTargetEnt = GetNextTarget();

		if (m_hTargetEnt == 0)
			return;

		pev->health = pev->max_health;
		Next();
	}
}