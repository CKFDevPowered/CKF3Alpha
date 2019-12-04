#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "saverestore.h"
#include "trains.h"
#include "gamerules.h"
#include "weapons.h"
#include "subs.h"
#include "trigger.h"

#define SF_TRIGGER_PUSH_START_OFF 2
#define SF_TRIGGER_HURT_TARGETONCE 1
#define SF_TRIGGER_HURT_START_OFF 2
#define SF_TRIGGER_HURT_NO_CLIENTS 8
#define SF_TRIGGER_HURT_CLIENTONLYFIRE 16
#define SF_TRIGGER_HURT_CLIENTONLYTOUCH 32

extern DLL_GLOBAL BOOL g_fGameOver;
extern void SetMovedir(entvars_t *pev);

class CFrictionModifier : public CBaseEntity
{
public:
	void Spawn(void);
	void KeyValue(KeyValueData *pkvd);
	int Save(CSave &save);
	int Restore(CRestore &restore);
	int ObjectCaps(void) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

public:
	void EXPORT ChangeFriction(CBaseEntity *pOther);

public:
	static TYPEDESCRIPTION m_SaveData[];

public:
	float m_frictionFraction;
};

LINK_ENTITY_TO_CLASS(func_friction, CFrictionModifier);

TYPEDESCRIPTION CFrictionModifier::m_SaveData[] =
{
	DEFINE_FIELD(CFrictionModifier, m_frictionFraction, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CFrictionModifier, CBaseEntity);

void CFrictionModifier::Spawn(void)
{
	pev->solid = SOLID_TRIGGER;
	SET_MODEL(ENT(pev), STRING(pev->model));
	pev->movetype = MOVETYPE_NONE;
	SetTouch(&CFrictionModifier::ChangeFriction);
}

void CFrictionModifier::ChangeFriction(CBaseEntity *pOther)
{
	if (pOther->pev->movetype != MOVETYPE_BOUNCEMISSILE && pOther->pev->movetype != MOVETYPE_BOUNCE)
		pOther->pev->friction = m_frictionFraction;
}

void CFrictionModifier::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "modifier"))
	{
		m_frictionFraction = atof(pkvd->szValue) / 100;
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

#define SF_AUTO_FIREONCE 0x0001

class CAutoTrigger : public CBaseDelay
{
public:
	void KeyValue(KeyValueData *pkvd);
	void Spawn(void);
	void Precache(void);
	void Think(void);
	int ObjectCaps(void) { return CBaseDelay::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

private:
	int m_globalstate;
	USE_TYPE triggerType;
};

LINK_ENTITY_TO_CLASS(trigger_auto, CAutoTrigger);

void CAutoTrigger::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "globalstate"))
	{
		m_globalstate = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "triggerstate"))
	{
		int type = atoi(pkvd->szValue);

		switch (type)
		{
			case 0: triggerType = USE_OFF; break;
			case 2: triggerType = USE_TOGGLE; break;
			default: triggerType = USE_ON; break;
		}

		pkvd->fHandled = TRUE;
	}
	else
		CBaseDelay::KeyValue(pkvd);
}

void CAutoTrigger::Spawn(void)
{
	Precache();
}

void CAutoTrigger::Precache(void)
{
	pev->nextthink = gpGlobals->time + 0.1;
}

void CAutoTrigger::Think(void)
{
	if (!m_globalstate || gGlobalState.EntityGetState(m_globalstate) == GLOBAL_ON)
	{
		SUB_UseTargets(this, triggerType, 0);

		if (pev->spawnflags & SF_AUTO_FIREONCE)
			UTIL_Remove(this);
	}
}

#define SF_RELAY_FIREONCE 0x0001

class CTriggerRelay : public CBaseDelay
{
public:
	void KeyValue(KeyValueData *pkvd);
	void Spawn(void);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	int ObjectCaps(void) { return CBaseDelay::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

private:
	USE_TYPE triggerType;
};

LINK_ENTITY_TO_CLASS(trigger_relay, CTriggerRelay);

void CTriggerRelay::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "triggerstate"))
	{
		int type = atoi(pkvd->szValue);

		switch (type)
		{
			case 0: triggerType = USE_OFF; break;
			case 2: triggerType = USE_TOGGLE; break;
			default: triggerType = USE_ON; break;
		}

		pkvd->fHandled = TRUE;
	}
	else
		CBaseDelay::KeyValue(pkvd);
}

void CTriggerRelay::Spawn(void)
{
}

void CTriggerRelay::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	SUB_UseTargets(this, triggerType, 0);

	if (pev->spawnflags & SF_RELAY_FIREONCE)
		UTIL_Remove(this);
}

#define SF_MULTIMAN_CLONE 0x80000000
#define SF_MULTIMAN_THREAD 0x00000001

class CMultiManager : public CBaseToggle
{
public:
	void KeyValue(KeyValueData *pkvd);
	void Spawn(void);
	void Restart(void);
	BOOL HasTarget(string_t targetname);
	int ObjectCaps(void) { return CBaseToggle::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

public:
	void EXPORT ManagerThink(void);
	void EXPORT ManagerUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

private:
	inline BOOL IsClone(void) { return (pev->spawnflags & SF_MULTIMAN_CLONE) ? TRUE : FALSE; }
	inline BOOL ShouldClone(void)
	{
		if (IsClone())
			return FALSE;

		return (pev->spawnflags & SF_MULTIMAN_THREAD) ? TRUE : FALSE;
	}

	CMultiManager *Clone(void);

public:
	int m_cTargets;
	int m_index;
	float m_startTime;
	int m_iTargetName[MAX_MULTI_TARGETS];
	float m_flTargetDelay[MAX_MULTI_TARGETS];
};

LINK_ENTITY_TO_CLASS(multi_manager, CMultiManager);

void CMultiManager::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
	{
		if (m_cTargets < MAX_MULTI_TARGETS)
		{
			char tmp[128];
			UTIL_StripToken(pkvd->szKeyName, tmp);
			m_iTargetName[m_cTargets] = ALLOC_STRING(tmp);
			m_flTargetDelay[m_cTargets++] = atof(pkvd->szValue);
			pkvd->fHandled = TRUE;
		}
	}
}

void CMultiManager::Spawn(void)
{
	pev->solid = SOLID_NOT;
	SetUse(&CMultiManager::ManagerUse);
	SetThink(&CMultiManager::ManagerThink);

	int swapped = 1;

	while (swapped)
	{
		swapped = 0;

		for (int i = 1; i < m_cTargets; i++)
		{
			if (m_flTargetDelay[i] < m_flTargetDelay[i-1])
			{
				int name = m_iTargetName[i];
				float delay = m_flTargetDelay[i];
				m_iTargetName[i] = m_iTargetName[i - 1];
				m_flTargetDelay[i] = m_flTargetDelay[i - 1];
				m_iTargetName[i - 1] = name;
				m_flTargetDelay[i - 1] = delay;
				swapped = 1;
			}
		}
	}
}

void CMultiManager::Restart(void)
{
	edict_t *pentTarget = NULL;

	for (int i = 0; i < m_cTargets; i++)
	{
		const char *name = STRING(m_iTargetName[i]);

		if (!name)
			continue;

		pentTarget = FIND_ENTITY_BY_TARGETNAME(pentTarget, STRING(pev->target));

		if (FNullEnt(pentTarget))
			break;

		CBaseEntity *pTarget = (CBaseEntity *)CBaseEntity::Instance(pentTarget);

		if (pTarget && !(pTarget->pev->flags & FL_KILLME))
			pTarget->Restart();
	}

	SetThink(NULL);

	if (IsClone())
	{
		UTIL_Remove(this);
		return;
	}

	SetUse(&CMultiManager::ManagerUse);
}

BOOL CMultiManager::HasTarget(string_t targetname)
{
	for (int i = 0; i < m_cTargets; i++)
		if (FStrEq(STRING(targetname), STRING(m_iTargetName[i])))
			return TRUE;

	return FALSE;
}

void CMultiManager::ManagerThink(void)
{
	float time = gpGlobals->time - m_startTime;

	while (m_index < m_cTargets && m_flTargetDelay[m_index] <= time)
	{
		FireTargets(STRING(m_iTargetName[m_index]), m_hActivator, this, USE_TOGGLE, 0);
		m_index++;
	}

	if (m_index >= m_cTargets)
	{
		SetThink(NULL);

		if (IsClone())
		{
			UTIL_Remove(this);
			return;
		}

		SetUse(&CMultiManager::ManagerUse);
	}
	else
		pev->nextthink = m_startTime + m_flTargetDelay[m_index];
}

CMultiManager *CMultiManager::Clone(void)
{
	CMultiManager *pMulti = GetClassPtr((CMultiManager *)NULL);
	edict_t *pEdict = pMulti->pev->pContainingEntity;
	memcpy(pMulti->pev, pev, sizeof(*pev));
	pMulti->pev->pContainingEntity = pEdict;
	pMulti->pev->spawnflags |= SF_MULTIMAN_CLONE;
	pMulti->m_cTargets = m_cTargets;
	memcpy(pMulti->m_iTargetName, m_iTargetName, sizeof(m_iTargetName));
	memcpy(pMulti->m_flTargetDelay, m_flTargetDelay, sizeof(m_flTargetDelay));
	return pMulti;
}

void CMultiManager::ManagerUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (ShouldClone())
	{
		CMultiManager *pClone = Clone();
		pClone->ManagerUse(pActivator, pCaller, useType, value);
		return;
	}

	m_hActivator = pActivator;
	m_index = 0;
	m_startTime = gpGlobals->time;

	SetUse(NULL);
	SetThink(&CMultiManager::ManagerThink);
	pev->nextthink = gpGlobals->time;
}

#define SF_RENDER_MASKFX (1<<0)
#define SF_RENDER_MASKAMT (1<<1)
#define SF_RENDER_MASKMODE (1<<2)
#define SF_RENDER_MASKCOLOR (1<<3)

class CRenderFxManager : public CBaseEntity
{
public:
	void Spawn(void);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
};

LINK_ENTITY_TO_CLASS(env_render, CRenderFxManager);

void CRenderFxManager::Spawn(void)
{
	pev->solid = SOLID_NOT;
}

void CRenderFxManager::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (FStringNull(pev->target))
		return;

	edict_t *pentTarget = NULL;

	while (1)
	{
		pentTarget = FIND_ENTITY_BY_TARGETNAME(pentTarget, STRING(pev->target));

		if (FNullEnt(pentTarget))
			break;

		entvars_t *pevTarget = VARS(pentTarget);

		if (!FBitSet(pev->spawnflags, SF_RENDER_MASKFX))
			pevTarget->renderfx = pev->renderfx;

		if (!FBitSet(pev->spawnflags, SF_RENDER_MASKAMT))
			pevTarget->renderamt = pev->renderamt;

		if (!FBitSet(pev->spawnflags, SF_RENDER_MASKMODE))
			pevTarget->rendermode = pev->rendermode;

		if (!FBitSet(pev->spawnflags, SF_RENDER_MASKCOLOR))
			pevTarget->rendercolor = pev->rendercolor;
	}
}

LINK_ENTITY_TO_CLASS(trigger, CBaseTrigger);

void CBaseTrigger::InitTrigger(void)
{
	if (pev->angles != g_vecZero)
		SetMovedir(pev);

	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_NONE;
	pev->effects |= EF_NODRAW;

	SET_MODEL(ENT(pev), STRING(pev->model));
}

void CBaseTrigger::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "damage"))
	{
		pev->dmg = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "count"))
	{
		m_cTriggersLeft = (int) atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "damagetype"))
	{
		m_bitsDamageInflict = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseToggle::KeyValue(pkvd);
}

class CTriggerHurt : public CBaseTrigger
{
public:
	void Spawn(void);
	void EXPORT RadiationThink(void);
};

LINK_ENTITY_TO_CLASS(trigger_hurt, CTriggerHurt);

void CTriggerHurt::Spawn(void)
{
	InitTrigger();
	SetTouch(&CBaseTrigger::HurtTouch);

	if (!FStringNull(pev->targetname))
		SetUse(&CBaseTrigger::ToggleUse);
	else
		SetUse(NULL);

	if (m_bitsDamageInflict & DMG_RADIATION)
	{
		SetThink(&CTriggerHurt::RadiationThink);
		pev->nextthink = gpGlobals->time + RANDOM_FLOAT(0, 0.5);
	}

	if (FBitSet(pev->spawnflags, SF_TRIGGER_HURT_START_OFF))
		pev->solid = SOLID_NOT;

	UTIL_SetOrigin(pev, pev->origin);
}

void CTriggerHurt::RadiationThink(void)
{
	Vector origin = pev->origin;
	Vector view_ofs = pev->view_ofs;

	pev->origin = (pev->absmin + pev->absmax) * 0.5;
	pev->view_ofs = pev->view_ofs * 0;

	edict_t *pentPlayer = FIND_CLIENT_IN_PVS(edict());

	pev->origin = origin;
	pev->view_ofs = view_ofs;

	if (!FNullEnt(pentPlayer))
	{
		CBasePlayer *pPlayer = GetClassPtr((CBasePlayer *)VARS(pentPlayer));
		entvars_t *pevTarget = VARS(pentPlayer);
		Vector vecSpot1 = (pev->absmin + pev->absmax) * 0.5;
		Vector vecSpot2 = (pevTarget->absmin + pevTarget->absmax) * 0.5;
		Vector vecRange = vecSpot1 - vecSpot2;
		float flRange = vecRange.Length();

		if (pPlayer->m_flgeigerRange >= flRange)
			pPlayer->m_flgeigerRange = flRange;
	}

	pev->nextthink = gpGlobals->time + 0.25;
}

void CBaseTrigger::ToggleUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (pev->solid == SOLID_NOT)
	{
		pev->solid = SOLID_TRIGGER;
		gpGlobals->force_retouch++;
	}
	else
		pev->solid = SOLID_NOT;

	UTIL_SetOrigin(pev, pev->origin);
}

void CBaseTrigger::HurtTouch (CBaseEntity *pOther)
{
	if (!pOther->pev->takedamage)
		return;

	if ((pev->spawnflags & SF_TRIGGER_HURT_CLIENTONLYTOUCH) && !pOther->IsPlayer())
		return;

	if ((pev->spawnflags & SF_TRIGGER_HURT_NO_CLIENTS) && pOther->IsPlayer())
		return;

	if (g_pGameRules->IsMultiplayer())
	{
		if (pev->dmgtime > gpGlobals->time)
		{
			if (gpGlobals->time != pev->pain_finished)
			{
				if (pOther->IsPlayer())
				{
					int playerMask = 1 << (pOther->entindex() - 1);

					if (pev->impulse & playerMask)
						return;

					pev->impulse |= playerMask;
				}
				else
					return;
			}
		}
		else
		{
			pev->impulse = 0;

			if (pOther->IsPlayer())
			{
				int playerMask = 1 << (pOther->entindex() - 1);
				pev->impulse |= playerMask;
			}
		}
	}
	else if (pev->dmgtime > gpGlobals->time && gpGlobals->time != pev->pain_finished)
		return;

	float fldmg = pev->dmg * 0.5;

	if (fldmg < 0)
		pOther->TakeHealth(-fldmg, m_bitsDamageInflict);
	else
		pOther->TakeDamage(pev, pev, fldmg, m_bitsDamageInflict, 0);

	pev->pain_finished = gpGlobals->time;
	pev->dmgtime = gpGlobals->time + 0.5;

	if (pev->target)
	{
		if (pev->spawnflags & SF_TRIGGER_HURT_CLIENTONLYFIRE)
		{
			if (!pOther->IsPlayer())
				return;
		}

		SUB_UseTargets(pOther, USE_TOGGLE, 0);

		if (pev->spawnflags & SF_TRIGGER_HURT_TARGETONCE)
			pev->target = 0;
	}
}

class CTriggerMultiple : public CBaseTrigger
{
public:
	void Spawn(void);
};

LINK_ENTITY_TO_CLASS(trigger_multiple, CTriggerMultiple);

void CTriggerMultiple::Spawn(void)
{
	if (!m_flWait)
		m_flWait = 0.2;

	InitTrigger();
	SetTouch(&CBaseTrigger::MultiTouch);
}

class CTriggerOnce : public CTriggerMultiple
{
public:
	void Spawn(void);
};

LINK_ENTITY_TO_CLASS(trigger_once, CTriggerOnce);

void CTriggerOnce::Spawn(void)
{
	m_flWait = -1;
	CTriggerMultiple::Spawn();
}

void CBaseTrigger::MultiTouch(CBaseEntity *pOther)
{
	entvars_t *pevToucher = pOther->pev;

	if (((pevToucher->flags & FL_CLIENT) && !(pev->spawnflags & SF_TRIGGER_NOCLIENTS)) || ((pevToucher->flags & FL_MONSTER) && (pev->spawnflags & SF_TRIGGER_ALLOWMONSTERS)) || (pev->spawnflags & SF_TRIGGER_PUSHABLES) && FClassnameIs(pevToucher,"func_pushable"))
		ActivateMultiTrigger(pOther);
}

void CBaseTrigger::ActivateMultiTrigger(CBaseEntity *pActivator)
{
	if (pev->nextthink > gpGlobals->time)
		return;

	if (!UTIL_IsMasterTriggered(m_sMaster, pActivator))
		return;

	if (FClassnameIs(pev, "trigger_secret"))
	{
		if (!pev->enemy || !FClassnameIs(pev->enemy, "player"))
			return;

		gpGlobals->found_secrets++;
	}

	if (!FStringNull(pev->noise))
		EMIT_SOUND(ENT(pev), CHAN_VOICE, (char *)STRING(pev->noise), 1, ATTN_NORM);

	m_hActivator = pActivator;
	SUB_UseTargets(m_hActivator, USE_TOGGLE, 0);

	if (pev->message && pActivator->IsPlayer())
		UTIL_ShowMessage(STRING(pev->message), pActivator);

	if (m_flWait > 0)
	{
		SetThink(&CBaseTrigger::MultiWaitOver);
		pev->nextthink = gpGlobals->time + m_flWait;
	}
	else
	{
		SetTouch(NULL);
		pev->nextthink = gpGlobals->time + 0.1;
		SetThink(&CBaseEntity::SUB_Remove);
	}
}

void CBaseTrigger::MultiWaitOver(void)
{
	SetThink(NULL);
}

void CBaseTrigger::CounterUse(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	m_cTriggersLeft--;
	m_hActivator = pActivator;

	if (m_cTriggersLeft < 0)
		return;

	BOOL fTellActivator = (m_hActivator != 0) && FClassnameIs(m_hActivator->pev, "player") && !FBitSet(pev->spawnflags, SPAWNFLAG_NOMESSAGE);

	if (m_cTriggersLeft)
	{
		if (fTellActivator)
		{
			switch (m_cTriggersLeft)
			{
				case 1: ALERT(at_console, "Only 1 more to go..."); break;
				case 2: ALERT(at_console, "Only 2 more to go..."); break;
				case 3: ALERT(at_console, "Only 3 more to go..."); break;
				default: ALERT(at_console, "There are more to go..."); break;
			}
		}

		return;
	}

	if (fTellActivator)
		ALERT(at_console, "Sequence completed!");

	ActivateMultiTrigger(m_hActivator);
}

class CTriggerCounter : public CBaseTrigger
{
public:
	void Spawn(void);
};

LINK_ENTITY_TO_CLASS(trigger_counter, CTriggerCounter);

void CTriggerCounter::Spawn(void)
{
	m_flWait = -1;

	if (!m_cTriggersLeft)
		m_cTriggersLeft = 2;

	SetUse(&CBaseTrigger::CounterUse);
}

class CTriggerVolume : public CPointEntity
{
public:
	void Spawn(void);
};

LINK_ENTITY_TO_CLASS(trigger_transition, CTriggerVolume);

void CTriggerVolume::Spawn(void)
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	SET_MODEL(ENT(pev), STRING(pev->model));
	pev->model = 0;
	pev->modelindex = 0;
}

#define SF_CHANGELEVEL_USEONLY 0x0002

class CChangeLevel : public CBaseTrigger
{
public:
	void Spawn(void);
	void KeyValue(KeyValueData *pkvd);
	int Save(CSave &save);
	int Restore(CRestore &restore);

public:
	void EXPORT UseChangeLevel(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void EXPORT TriggerChangeLevel(void);
	void EXPORT ExecuteChangeLevel(void);
	void EXPORT TouchChangeLevel(CBaseEntity *pOther);
	void ChangeLevelNow(CBaseEntity *pActivator);
	static edict_t *FindLandmark(const char *pLandmarkName);
	static int ChangeList(LEVELLIST *pLevelList, int maxList);
	static int AddTransitionToList(LEVELLIST *pLevelList, int listCount, const char *pMapName, const char *pLandmarkName, edict_t *pentLandmark);
	static int InTransitionVolume(CBaseEntity *pEntity, char *pVolumeName);

public:
	static TYPEDESCRIPTION m_SaveData[];

public:
	char m_szMapName[cchMapNameMost];
	char m_szLandmarkName[cchMapNameMost];
	int m_changeTarget;
	float m_changeTargetDelay;
};

LINK_ENTITY_TO_CLASS(trigger_changelevel, CChangeLevel);

TYPEDESCRIPTION CChangeLevel::m_SaveData[] =
{
	DEFINE_ARRAY(CChangeLevel, m_szMapName, FIELD_CHARACTER, cchMapNameMost),
	DEFINE_ARRAY(CChangeLevel, m_szLandmarkName, FIELD_CHARACTER, cchMapNameMost),
	DEFINE_FIELD(CChangeLevel, m_changeTarget, FIELD_STRING),
	DEFINE_FIELD(CChangeLevel, m_changeTargetDelay, FIELD_FLOAT),
};

IMPLEMENT_SAVERESTORE(CChangeLevel, CBaseTrigger);

void CChangeLevel::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "map"))
	{
		if (strlen(pkvd->szValue) >= cchMapNameMost)
			ALERT(at_error, "Map name '%s' too long (32 chars)\n", pkvd->szValue);

		strcpy(m_szMapName, pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "landmark"))
	{
		if (strlen(pkvd->szValue) >= cchMapNameMost)
			ALERT(at_error, "Landmark name '%s' too long (32 chars)\n", pkvd->szValue);

		strcpy(m_szLandmarkName, pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "changetarget"))
	{
		m_changeTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "changedelay"))
	{
		m_changeTargetDelay = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseTrigger::KeyValue(pkvd);
}

void CChangeLevel::Spawn(void)
{
	if (FStrEq(m_szMapName, ""))
		ALERT(at_console, "a trigger_changelevel doesn't have a map");

	if (FStrEq(m_szLandmarkName, ""))
		ALERT(at_console, "trigger_changelevel to %s doesn't have a landmark", m_szMapName);

	if (!FStringNull(pev->targetname))
		SetUse(&CChangeLevel::UseChangeLevel);

	InitTrigger();

	if (!(pev->spawnflags & SF_CHANGELEVEL_USEONLY))
		SetTouch(&CChangeLevel::TouchChangeLevel);
}

void CChangeLevel::ExecuteChangeLevel(void)
{
	MESSAGE_BEGIN(MSG_ALL, SVC_CDTRACK);
	WRITE_BYTE(3);
	WRITE_BYTE(3);
	MESSAGE_END();

	MESSAGE_BEGIN(MSG_ALL, SVC_INTERMISSION);
	MESSAGE_END();
}

FILE_GLOBAL char st_szNextMap[cchMapNameMost];
FILE_GLOBAL char st_szNextSpot[cchMapNameMost];

edict_t *CChangeLevel::FindLandmark(const char *pLandmarkName)
{
	edict_t *pentLandmark = FIND_ENTITY_BY_STRING(NULL, "targetname", pLandmarkName);

	while (!FNullEnt(pentLandmark))
	{
		if (FClassnameIs(pentLandmark, "info_landmark"))
			return pentLandmark;
		else
			pentLandmark = FIND_ENTITY_BY_STRING(pentLandmark, "targetname", pLandmarkName);
	}

	ALERT(at_error, "Can't find landmark %s\n", pLandmarkName);
	return NULL;
}

void CChangeLevel::UseChangeLevel(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	ChangeLevelNow(pActivator);
}

void CChangeLevel::ChangeLevelNow(CBaseEntity *pActivator)
{
	if (g_pGameRules->IsDeathmatch())
		return;

	if (gpGlobals->time == pev->dmgtime)
		return;

	pev->dmgtime = gpGlobals->time;
	CBaseEntity *pPlayer = CBaseEntity::Instance(g_engfuncs.pfnPEntityOfEntIndex(1));

	if (!InTransitionVolume(pPlayer, m_szLandmarkName))
	{
		ALERT(at_aiconsole, "Player isn't in the transition volume %s, aborting\n", m_szLandmarkName);
		return;
	}

	if (m_changeTarget)
	{
/*		CFireAndDie *pFireAndDie = GetClassPtr((CFireAndDie *)NULL);

		if (pFireAndDie)
		{
			pFireAndDie->pev->target = m_changeTarget;
			pFireAndDie->m_flDelay = m_changeTargetDelay;
			pFireAndDie->pev->origin = pPlayer->pev->origin;
			DispatchSpawn(pFireAndDie->edict());
		}*/
	}

	strcpy(st_szNextMap, m_szMapName);
	m_hActivator = pActivator;
	SUB_UseTargets(pActivator, USE_TOGGLE, 0);
	st_szNextSpot[0] = '\0';

	edict_t *pentLandmark = FindLandmark(m_szLandmarkName);

	if (!FNullEnt(pentLandmark))
	{
		strcpy(st_szNextSpot, m_szLandmarkName);
		gpGlobals->vecLandmarkOffset = VARS(pentLandmark)->origin;
	}

	ALERT(at_console, "CHANGE LEVEL: %s %s\n", st_szNextMap, st_szNextSpot);
	CHANGE_LEVEL(st_szNextMap, st_szNextSpot);
}

void CChangeLevel::TouchChangeLevel(CBaseEntity *pOther)
{
	if (!FClassnameIs(pOther->pev, "player"))
		return;

	ChangeLevelNow(pOther);
}

int CChangeLevel::AddTransitionToList(LEVELLIST *pLevelList, int listCount, const char *pMapName, const char *pLandmarkName, edict_t *pentLandmark)
{
	if (!pLevelList || !pMapName || !pLandmarkName || !pentLandmark)
		return 0;

	for (int i = 0; i < listCount; i++)
	{
		if (pLevelList[i].pentLandmark == pentLandmark && !strcmp(pLevelList[i].mapName, pMapName))
			return 0;
	}

	strcpy(pLevelList[listCount].mapName, pMapName);
	strcpy(pLevelList[listCount].landmarkName, pLandmarkName);
	pLevelList[listCount].pentLandmark = pentLandmark;
	pLevelList[listCount].vecLandmarkOrigin = VARS(pentLandmark)->origin;

	return 1;
}

int BuildChangeList(LEVELLIST *pLevelList, int maxList)
{
	return CChangeLevel::ChangeList(pLevelList, maxList);
}

int CChangeLevel::InTransitionVolume(CBaseEntity *pEntity, char *pVolumeName)
{
	if (pEntity->ObjectCaps() & FCAP_FORCE_TRANSITION)
		return 1;

	if (pEntity->pev->movetype == MOVETYPE_FOLLOW)
	{
		if (pEntity->pev->aiment)
			pEntity = CBaseEntity::Instance(pEntity->pev->aiment);
	}

	int inVolume = 1;
	edict_t *pentVolume = FIND_ENTITY_BY_TARGETNAME(NULL, pVolumeName);

	while (!FNullEnt(pentVolume))
	{
		CBaseEntity *pVolume = CBaseEntity::Instance(pentVolume);

		if (pVolume && FClassnameIs(pVolume->pev, "trigger_transition"))
		{
			if (pVolume->Intersects(pEntity))
				return 1;

			inVolume = 0;
		}

		pentVolume = FIND_ENTITY_BY_TARGETNAME(pentVolume, pVolumeName);
	}

	return inVolume;
}

#define MAX_ENTITY 512

int CChangeLevel::ChangeList(LEVELLIST *pLevelList, int maxList)
{
	int count = 0;
	edict_t *pentChangelevel = FIND_ENTITY_BY_STRING(NULL, "classname", "trigger_changelevel");

	if (FNullEnt(pentChangelevel))
		return 0;

	while (!FNullEnt(pentChangelevel))
	{
		CChangeLevel *pTrigger = GetClassPtr((CChangeLevel *)VARS(pentChangelevel));

		if (pTrigger)
		{
			edict_t *pentLandmark = FindLandmark(pTrigger->m_szLandmarkName);

			if (pentLandmark)
			{
				if (AddTransitionToList(pLevelList, count, pTrigger->m_szMapName, pTrigger->m_szLandmarkName, pentLandmark))
				{
					count++;

					if (count >= maxList)
						break;
				}
			}
		}

		pentChangelevel = FIND_ENTITY_BY_STRING(pentChangelevel, "classname", "trigger_changelevel");
	}

	if (gpGlobals->pSaveData && ((SAVERESTOREDATA *)gpGlobals->pSaveData)->pTable)
	{
		CSave saveHelper((SAVERESTOREDATA *)gpGlobals->pSaveData);

		for (int i = 0; i < count; i++)
		{
			int entityCount = 0;
			CBaseEntity *pEntList[MAX_ENTITY];
			int entityFlags[MAX_ENTITY];
			edict_t *pent = UTIL_EntitiesInPVS(pLevelList[i].pentLandmark);

			while (!FNullEnt(pent))
			{
				CBaseEntity *pEntity = CBaseEntity::Instance(pent);

				if (pEntity)
				{
					int caps = pEntity->ObjectCaps();

					if (!(caps & FCAP_DONT_SAVE))
					{
						int flags = 0;

						if (caps & FCAP_ACROSS_TRANSITION)
							flags |= FENTTABLE_MOVEABLE;

						if (pEntity->pev->globalname && !pEntity->IsDormant())
							flags |= FENTTABLE_GLOBAL;

						if (flags)
						{
							pEntList[entityCount] = pEntity;
							entityFlags[entityCount] = flags;
							entityCount++;

							if (entityCount > MAX_ENTITY)
								ALERT(at_error, "Too many entities across a transition!");
						}
					}
				}

				pent = pent->v.chain;
			}

			for (int j = 0; j < entityCount; j++)
			{
				if (entityFlags[j] && InTransitionVolume(pEntList[j], pLevelList[i].landmarkName))
				{
					int index = saveHelper.EntityIndex(pEntList[j]);
					saveHelper.EntityFlagsSet(index, entityFlags[j] | (1 << i));
				}
			}
		}
	}

	return count;
}

void NextLevel(void)
{
	CChangeLevel *pChange;
	edict_t *pent = FIND_ENTITY_BY_CLASSNAME(NULL, "trigger_changelevel");

	if (FNullEnt(pent))
	{
		gpGlobals->mapname = ALLOC_STRING("start");
		pChange = GetClassPtr((CChangeLevel *)NULL);
		strcpy(pChange->m_szMapName, "start");
	}
	else
		pChange = GetClassPtr((CChangeLevel *)VARS(pent));

	strcpy(st_szNextMap, pChange->m_szMapName);
	g_fGameOver = TRUE;

	if (pChange->pev->nextthink < gpGlobals->time)
	{
		pChange->SetThink(&CChangeLevel::ExecuteChangeLevel);
		pChange->pev->nextthink = gpGlobals->time + 0.1;
	}
}

class CLadder : public CBaseTrigger
{
public:
	void KeyValue(KeyValueData *pkvd);
	void Spawn(void);
	void Precache(void);
};

LINK_ENTITY_TO_CLASS(func_ladder, CLadder);

void CLadder::KeyValue(KeyValueData *pkvd)
{
	CBaseTrigger::KeyValue(pkvd);
}

void CLadder::Precache(void)
{
	pev->solid = SOLID_NOT;
	pev->skin = CONTENTS_LADDER;

	if (CVAR_GET_FLOAT("showtriggers") == 0)
	{
		pev->rendermode = kRenderTransTexture;
		pev->renderamt = 0;
	}

	pev->effects &= ~EF_NODRAW;
}

void CLadder::Spawn(void)
{
	Precache();
	SET_MODEL(ENT(pev), STRING(pev->model));
	pev->movetype = MOVETYPE_PUSH;
}

class CTriggerPush : public CBaseTrigger
{
public:
	void Spawn(void);
	void KeyValue(KeyValueData *pkvd);
	void Touch(CBaseEntity *pOther);
};

LINK_ENTITY_TO_CLASS(trigger_push, CTriggerPush);

void CTriggerPush::KeyValue(KeyValueData *pkvd)
{
	CBaseTrigger::KeyValue(pkvd);
}

void CTriggerPush::Spawn(void)
{
	if (pev->angles == g_vecZero)
		pev->angles.y = 360;

	InitTrigger();

	if (!pev->speed)
		pev->speed = 100;

	if (FBitSet(pev->spawnflags, SF_TRIGGER_PUSH_START_OFF))
		pev->solid = SOLID_NOT;

	SetUse(&CBaseTrigger::ToggleUse);
	UTIL_SetOrigin(pev, pev->origin);
}

void CTriggerPush::Touch(CBaseEntity *pOther)
{
	entvars_t *pevToucher = pOther->pev;

	switch (pevToucher->movetype)
	{
		case MOVETYPE_NONE:
		case MOVETYPE_PUSH:
		case MOVETYPE_NOCLIP:
		case MOVETYPE_FOLLOW: return;
	}

	if (pevToucher->solid != SOLID_NOT && pevToucher->solid != SOLID_BSP)
	{
		if (FBitSet(pev->spawnflags, SF_TRIG_PUSH_ONCE))
		{
			pevToucher->velocity = pevToucher->velocity + (pev->speed * pev->movedir);

			if (pevToucher->velocity.z > 0)
				pevToucher->flags &= ~FL_ONGROUND;

			UTIL_Remove(this);
		}
		else
		{
			Vector vecPush = (pev->speed * pev->movedir);

			if (pevToucher->flags & FL_BASEVELOCITY)
				vecPush = vecPush + pevToucher->basevelocity;

			pevToucher->basevelocity = vecPush;
			pevToucher->flags |= FL_BASEVELOCITY;
		}
	}
}

void CBaseTrigger::TeleportTouch(CBaseEntity *pOther)
{
	entvars_t *pevToucher = pOther->pev;

	if (!FBitSet(pevToucher->flags, FL_CLIENT | FL_MONSTER))
		return;

	if (!UTIL_IsMasterTriggered(m_sMaster, pOther))
		return;

	if (!(pev->spawnflags & SF_TRIGGER_ALLOWMONSTERS))
	{
		if (FBitSet(pevToucher->flags, FL_MONSTER))
			return;
	}

	if ((pev->spawnflags & SF_TRIGGER_NOCLIENTS))
	{
		if (pOther->IsPlayer())
			return;
	}

	edict_t *pentTarget = FIND_ENTITY_BY_TARGETNAME(NULL, STRING(pev->target));

	if (FNullEnt(pentTarget))
		return;

	Vector tmp = VARS(pentTarget)->origin;

	if (pOther->IsPlayer())
		tmp.z -= pOther->pev->mins.z;

	tmp.z++;
	pevToucher->flags &= ~FL_ONGROUND;
	UTIL_SetOrigin(pevToucher, tmp);
	pevToucher->angles = pentTarget->v.angles;

	if (pOther->IsPlayer())
		pevToucher->v_angle = pentTarget->v.angles;

	pevToucher->fixangle = TRUE;
	pevToucher->velocity = pevToucher->basevelocity = g_vecZero;
}

class CTriggerTeleport : public CBaseTrigger
{
public:
	void Spawn(void);
};

LINK_ENTITY_TO_CLASS(trigger_teleport, CTriggerTeleport);

void CTriggerTeleport::Spawn(void)
{
	InitTrigger();
	SetTouch(&CBaseTrigger::TeleportTouch);
}

LINK_ENTITY_TO_CLASS(info_teleport_destination, CPointEntity);

class CTriggerGravity : public CBaseTrigger
{
public:
	void Spawn(void);
	void EXPORT GravityTouch(CBaseEntity *pOther);
};

LINK_ENTITY_TO_CLASS(trigger_gravity, CTriggerGravity);

void CTriggerGravity::Spawn(void)
{
	InitTrigger();
	SetTouch(&CTriggerGravity::GravityTouch);
}

void CTriggerGravity::GravityTouch(CBaseEntity *pOther)
{
	if (!pOther->IsPlayer())
		return;

	pOther->pev->gravity = pev->gravity;
}

class CTriggerChangeTarget : public CBaseDelay
{
public:
	void KeyValue(KeyValueData *pkvd);
	void Spawn(void);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	int ObjectCaps(void) { return CBaseDelay::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	int Save(CSave &save);
	int Restore(CRestore &restore);

public:
	static TYPEDESCRIPTION m_SaveData[];

private:
	int m_iszNewTarget;
};

LINK_ENTITY_TO_CLASS(trigger_changetarget, CTriggerChangeTarget);

TYPEDESCRIPTION CTriggerChangeTarget::m_SaveData[] =
{
	DEFINE_FIELD(CTriggerChangeTarget, m_iszNewTarget, FIELD_STRING),
};

IMPLEMENT_SAVERESTORE(CTriggerChangeTarget, CBaseDelay);

void CTriggerChangeTarget::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "m_iszNewTarget"))
	{
		m_iszNewTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseDelay::KeyValue(pkvd);
}

void CTriggerChangeTarget::Spawn(void)
{
}

void CTriggerChangeTarget::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	CBaseEntity *pTarget = UTIL_FindEntityByString(NULL, "targetname", STRING(pev->target));

	if (pTarget)
	{
		pTarget->pev->target = m_iszNewTarget;
		CBaseMonster *pMonster = pTarget->MyMonsterPointer();

		if (pMonster)
			pMonster->m_pGoalEnt = NULL;
	}
}

#define SF_CAMERA_PLAYER_POSITION 1
#define SF_CAMERA_PLAYER_TARGET 2
#define SF_CAMERA_PLAYER_TAKECONTROL 4

class CTriggerCamera : public CBaseDelay
{
public:
	void Spawn(void);
	void KeyValue(KeyValueData *pkvd);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	int Save(CSave &save);
	int Restore(CRestore &restore);
	int ObjectCaps(void) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

public:
	void EXPORT FollowTarget(void);
	void Move(void);

public:
	static TYPEDESCRIPTION m_SaveData[];

public:
	EHANDLE m_hPlayer;
	EHANDLE m_hTarget;
	CBaseEntity *m_pentPath;
	int m_sPath;
	float m_flWait;
	float m_flReturnTime;
	float m_flStopTime;
	float m_moveDistance;
	float m_targetSpeed;
	float m_initialSpeed;
	float m_acceleration;
	float m_deceleration;
	int m_state;
};
LINK_ENTITY_TO_CLASS(trigger_camera, CTriggerCamera);

TYPEDESCRIPTION CTriggerCamera::m_SaveData[] =
{
	DEFINE_FIELD(CTriggerCamera, m_hPlayer, FIELD_EHANDLE),
	DEFINE_FIELD(CTriggerCamera, m_hTarget, FIELD_EHANDLE),
	DEFINE_FIELD(CTriggerCamera, m_pentPath, FIELD_CLASSPTR),
	DEFINE_FIELD(CTriggerCamera, m_sPath, FIELD_STRING),
	DEFINE_FIELD(CTriggerCamera, m_flWait, FIELD_FLOAT),
	DEFINE_FIELD(CTriggerCamera, m_flReturnTime, FIELD_TIME),
	DEFINE_FIELD(CTriggerCamera, m_flStopTime, FIELD_TIME),
	DEFINE_FIELD(CTriggerCamera, m_moveDistance, FIELD_FLOAT),
	DEFINE_FIELD(CTriggerCamera, m_targetSpeed, FIELD_FLOAT),
	DEFINE_FIELD(CTriggerCamera, m_initialSpeed, FIELD_FLOAT),
	DEFINE_FIELD(CTriggerCamera, m_acceleration, FIELD_FLOAT),
	DEFINE_FIELD(CTriggerCamera, m_deceleration, FIELD_FLOAT),
	DEFINE_FIELD(CTriggerCamera, m_state, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CTriggerCamera, CBaseDelay);

void CTriggerCamera::Spawn(void)
{
	pev->movetype = MOVETYPE_NOCLIP;
	pev->solid = SOLID_NOT;
	pev->renderamt = 0;
	pev->rendermode = kRenderTransTexture;
	m_initialSpeed = pev->speed;

	if (!m_acceleration)
		m_acceleration = 500;

	if (!m_deceleration)
		m_deceleration = 500;
}

void CTriggerCamera::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "moveto"))
	{
		m_sPath = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "acceleration"))
	{
		m_acceleration = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "deceleration"))
	{
		m_deceleration = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseDelay::KeyValue(pkvd);
}

void CTriggerCamera::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (!ShouldToggle(useType, m_state))
		return;

	m_state = !m_state;

	if (!m_state)
	{
		m_flReturnTime = gpGlobals->time;
		return;
	}

	if (!pActivator || !pActivator->IsPlayer())
		pActivator = CBaseEntity::Instance(g_engfuncs.pfnPEntityOfEntIndex(1));

	m_hPlayer = pActivator;
	m_flReturnTime = gpGlobals->time + m_flWait;
	pev->speed = m_initialSpeed;
	m_targetSpeed = m_initialSpeed;

	if (FBitSet(pev->spawnflags, SF_CAMERA_PLAYER_TARGET))
		m_hTarget = m_hPlayer;
	else
		m_hTarget = GetNextTarget();

	if (m_hTarget == NULL)
		return;

	if (FBitSet(pev->spawnflags, SF_CAMERA_PLAYER_TAKECONTROL))
		((CBasePlayer *)pActivator)->EnableControl(FALSE);

	if (m_sPath)
		m_pentPath = Instance(FIND_ENTITY_BY_TARGETNAME(NULL, STRING(m_sPath)));
	else
		m_pentPath = NULL;

	m_flStopTime = gpGlobals->time;

	if (m_pentPath)
	{
		if (m_pentPath->pev->speed != 0)
			m_targetSpeed = m_pentPath->pev->speed;

		m_flStopTime += m_pentPath->GetDelay();
	}

	if (FBitSet(pev->spawnflags, SF_CAMERA_PLAYER_POSITION))
	{
		UTIL_SetOrigin(pev, pActivator->pev->origin + pActivator->pev->view_ofs);
		pev->angles.x = -pActivator->pev->angles.x;
		pev->angles.y = pActivator->pev->angles.y;
		pev->angles.z = 0;
		pev->velocity = pActivator->pev->velocity;
	}
	else
		pev->velocity = Vector(0, 0, 0);

	SET_VIEW(pActivator->edict(), edict());
	SET_MODEL(ENT(pev), STRING(pActivator->pev->model));

	SetThink(&CTriggerCamera::FollowTarget);
	pev->nextthink = gpGlobals->time;
	m_moveDistance = 0;
	Move();
}

void CTriggerCamera::FollowTarget(void)
{
	if (m_hPlayer == NULL)
		return;

	if (m_hTarget == NULL || m_flReturnTime < gpGlobals->time)
	{
		if (m_hPlayer->IsAlive())
		{
			SET_VIEW(m_hPlayer->edict(), m_hPlayer->edict());
			((CBasePlayer *)((CBaseEntity *)m_hPlayer))->EnableControl(TRUE);
		}

		SUB_UseTargets(this, USE_TOGGLE, 0);
		pev->avelocity = Vector(0, 0, 0);
		m_state = 0;
		return;
	}

	Vector vecGoal = UTIL_VecToAngles(m_hTarget->pev->origin - pev->origin);
	vecGoal.x = -vecGoal.x;

	if (pev->angles.y > 360)
		pev->angles.y -= 360;

	if (pev->angles.y < 0)
		pev->angles.y += 360;

	float dx = vecGoal.x - pev->angles.x;
	float dy = vecGoal.y - pev->angles.y;

	if (dx < -180)
		dx += 360;

	if (dx > 180)
		dx = dx - 360;

	if (dy < -180)
		dy += 360;

	if (dy > 180)
		dy = dy - 360;

	pev->avelocity.x = dx * 40 * gpGlobals->frametime;
	pev->avelocity.y = dy * 40 * gpGlobals->frametime;

	if (!(FBitSet(pev->spawnflags, SF_CAMERA_PLAYER_TAKECONTROL)))
	{
		pev->velocity = pev->velocity * 0.8;

		if (pev->velocity.Length() < 10)
			pev->velocity = g_vecZero;
	}

	pev->nextthink = gpGlobals->time;
	Move();
}

void CTriggerCamera::Move(void)
{
	if (!m_pentPath)
		return;

	m_moveDistance -= pev->speed * gpGlobals->frametime;

	if (m_moveDistance <= 0)
	{
		if (m_pentPath->pev->message)
		{
			FireTargets(STRING(m_pentPath->pev->message), this, this, USE_TOGGLE, 0);

			if (FBitSet(m_pentPath->pev->spawnflags, SF_CORNER_FIREONCE))
				m_pentPath->pev->message = 0;
		}

		m_pentPath = m_pentPath->GetNextTarget();

		if (!m_pentPath)
		{
			pev->velocity = g_vecZero;
		}
		else
		{
			if (m_pentPath->pev->speed)
				m_targetSpeed = m_pentPath->pev->speed;

			Vector delta = m_pentPath->pev->origin - pev->origin;
			m_moveDistance = delta.Length();
			pev->movedir = delta.Normalize();
			m_flStopTime = gpGlobals->time + m_pentPath->GetDelay();
		}
	}

	if (m_flStopTime > gpGlobals->time)
		pev->speed = UTIL_Approach(0, pev->speed, m_deceleration * gpGlobals->frametime);
	else
		pev->speed = UTIL_Approach(m_targetSpeed, pev->speed, m_acceleration * gpGlobals->frametime);

	float fraction = 2 * gpGlobals->frametime;
	pev->velocity = ((pev->movedir * pev->speed) * fraction) + (pev->velocity * (1 - fraction));
}

//hz new trigger entity

class CTriggerSpawnControl : public CBaseDelay
{
public:
	void Spawn(void);
	void KeyValue(KeyValueData *pkvd);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	int ObjectCaps(void) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

public:
	int m_iAction;
};
LINK_ENTITY_TO_CLASS(trigger_spawncontrol, CTriggerSpawnControl);

void CTriggerSpawnControl::Spawn(void)
{
	pev->solid = SOLID_NOT;
	m_iAction = 0;
}

void CTriggerSpawnControl::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "action"))
	{
		m_iAction = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseDelay::KeyValue(pkvd);
}

void CTriggerSpawnControl::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	CBaseEntity *pEntity = NULL;

	while ((pEntity = UTIL_FindEntityByTargetname(pEntity, STRING(pev->target))) != NULL)
	{
		if( strcmp( STRING(pev->classname), "info_player_start" ) && strcmp( STRING(pev->classname), "info_player_deathmatch" ) ) continue;
		switch(m_iAction)
		{
		case 0:case 1 :
			((CBaseDMStart *)pEntity)->m_iDisabled = m_iAction;break;
		default:
			((CBaseDMStart *)pEntity)->m_iDisabled = 1-((CBaseDMStart *)pEntity)->m_iDisabled;break;
		}
	}
}

class CResupplyRoom : public CBaseTrigger
{
public:
	void Spawn(void);
	void EXPORT ResupplyTouch(CBaseEntity *pOther);
};

LINK_ENTITY_TO_CLASS(func_resupplyroom, CResupplyRoom);

void CResupplyRoom::Spawn(void)
{
	InitTrigger();
	SetTouch(&CResupplyRoom::ResupplyTouch);

	if (pev->team > TEAM_CT || pev->team < TEAM_UNASSIGNED)
	{
		ALERT(at_console, "Bad team number (%i) in %s\n", pev->team, STRING(pev->classname));
		pev->team = 0;
	}
}

void CResupplyRoom::ResupplyTouch(CBaseEntity *pOther)
{
	if (!pOther->IsPlayer())
		return;
	if (!pOther->IsAlive())
		return;

	CBasePlayer *pPlayer = (CBasePlayer *)pOther;

	if (pev->team == TEAM_UNASSIGNED || pev->team == pPlayer->m_iTeam)
		pPlayer->m_iMapZone |= MAPZONE_RESUPPLYROOM;
}

class CResupplyCase : public CBaseTrigger
{
public:
	void Spawn(void);
	void ResupplyTouch(CBaseEntity *pOther);
	void KeyValue(KeyValueData *pkvd);
	void AnimThink(void);
	void PostSpawn(void);
public:
	CBaseEntity *m_pSubEntity;
	string_t m_iSubEntity;
	int m_iAnim;
};

#define RESUPPLY_IDLE 0
#define RESUPPLY_OPEN 1
#define RESUPPLY_OPENIDLE 2
#define RESUPPLY_CLOSE 3

void CResupplyCase::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "subentity"))
	{
		m_iSubEntity = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	return CBaseTrigger::KeyValue(pkvd);
}

LINK_ENTITY_TO_CLASS(func_resupplycase, CResupplyCase);

void CResupplyCase::Spawn(void)
{
	InitTrigger();
	SetTouch(&CResupplyCase::ResupplyTouch);

	if (pev->team > TEAM_CT || pev->team < TEAM_UNASSIGNED)
	{
		ALERT(at_console, "Bad team number (%i) in %s\n", pev->team, STRING(pev->classname));
		pev->team = 0;
	}
	m_iAnim = RESUPPLY_IDLE;
}

void CResupplyCase::PostSpawn(void)
{
	m_pSubEntity = UTIL_FindEntityByTargetname(NULL, STRING(m_iSubEntity));
	if(m_pSubEntity)
	{
		m_pSubEntity->pev->sequence = 0;
		m_pSubEntity->pev->framerate = 1.0;
	}
}

void CResupplyCase::ResupplyTouch(CBaseEntity *pOther)
{
	if (!pOther->IsPlayer())
		return;
	if (!pOther->IsAlive())
		return;

	CBasePlayer *pPlayer = (CBasePlayer *)pOther;

	if (pev->team == TEAM_UNASSIGNED || pev->team == pPlayer->m_iTeam)
	{
		if(pPlayer->ResupplyCase())
		{
			if(m_pSubEntity)
			{
				pev->nextthink = gpGlobals->time + 0.5;
				SetThink(&CResupplyCase::AnimThink);
				m_iAnim = RESUPPLY_OPEN;
				if(m_pSubEntity) {m_pSubEntity->pev->sequence = m_iAnim;m_pSubEntity->pev->animtime = gpGlobals->time;}	
			}
		}
	}
}

void CResupplyCase::AnimThink(void)
{
	if(m_iAnim == RESUPPLY_OPEN)
	{
		pev->nextthink = gpGlobals->time + 3.0;
		m_iAnim = RESUPPLY_OPENIDLE;
	}
	else if(m_iAnim == RESUPPLY_OPENIDLE)
	{
		pev->nextthink = gpGlobals->time + 0.5;
		m_iAnim = RESUPPLY_CLOSE;
	}
	else if(m_iAnim == RESUPPLY_CLOSE)
	{
		m_iAnim = RESUPPLY_IDLE;
	}
	if(m_pSubEntity)
	{
		m_pSubEntity->pev->sequence = m_iAnim;
		m_pSubEntity->pev->animtime = gpGlobals->time;
	}	
}

LINK_ENTITY_TO_CLASS(func_controlpoint, CControlPoint);

void CControlPoint::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "subentity"))
	{
		m_iSubEntity = ALLOC_STRING( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "capname"))
	{
		strncpy(m_szName, pkvd->szValue, 31);
		m_szName[31] = 0;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "caprate"))
	{
		m_flCaptureRate = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "hudpos"))
	{
		m_iHUDPosition = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "trigonredcap"))
	{
		m_iTrigOnRedCap = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "trigonblucap"))
	{
		m_iTrigOnBluCap = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "trigoncap"))
	{
		m_iTrigOnCap = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "timeadded"))
	{
		m_iTimeAdded = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "locked"))
	{
		m_bOriginLocked = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "disabled"))
	{
		m_bOriginDisabled = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "canredcap"))
	{
		m_bOriginCanRedCap = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "canblucap"))
	{
		m_bOriginCanBluCap = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

void CControlPoint::PostSpawn(void)
{
	m_pSubEntity = UTIL_FindEntityByTargetname(NULL, STRING(m_iSubEntity));
	if(m_pSubEntity)
	{
		m_pSubEntity->pev->sequence = 0;
		m_pSubEntity->pev->framerate = 1.0;
		m_pSubEntity->pev->skin = pev->team;
	}
	m_iOriginTeam = pev->team;
	if(!m_szName[0])
	{
		strcpy(m_szName, "Control Point");
	}
	m_iIndex = g_pGameRules->m_ControlPoints.Count() + 1;
	g_pGameRules->m_ControlPoints.AddToTail(edict());
}

void CControlPoint::Restart(void)
{
	pev->team = m_iOriginTeam;
	if(m_pSubEntity)
	{
		m_pSubEntity->pev->skin = pev->team;
	}
	m_iState = 0;
	m_bLocked = m_bOriginLocked;
	m_bDisabled = m_bOriginDisabled;
	m_bCanRedCap = m_bOriginCanRedCap;
	m_bCanBluCap = m_bOriginCanBluCap;
	m_flProgress = 0;
	m_iCapTeam = 0;
}

void CControlPoint::Spawn(void)
{
	InitTrigger();
	SetTouch(&CControlPoint::ControlPointTouch);
	SetThink(&CControlPoint::ControlPointThink);
	pev->nextthink = gpGlobals->time + 1.0;
	m_iState = 0;
	m_flProgress = 0;
}

void CControlPoint::ControlPointCaptured(int iNewTeam)
{
	int iOldTeam = pev->team;
	pev->team = iNewTeam;
	m_flProgress = 0;
	m_iState = CP_IDLE;
	m_iCapTeam = 0;

	if(m_pSubEntity)//Use new team color skin
	{
		m_pSubEntity->pev->skin = pev->team;
	}

	//Need to add the timer?
	int iTimeAdded = m_iTimeAdded;

	//We trigger first
	if(iNewTeam == TEAM_RED && m_iTrigOnRedCap)
		FireTargets(STRING(m_iTrigOnRedCap), this, this, USE_TOGGLE, 0);
	else if(iNewTeam == TEAM_BLU && m_iTrigOnBluCap)
		FireTargets(STRING(m_iTrigOnBluCap), this, this, USE_TOGGLE, 0);
	else if(m_iTrigOnCap)
		FireTargets(STRING(m_iTrigOnCap), this, this, USE_TOGGLE, 0);

	//Check win and update hud
	//UpdateHUD();
	g_pGameRules->CheckWinConditions();

	int iCapturer = 0;
	CBasePlayer *plCapturer[32];
	//Players event
	for(int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex(i);
		if(!pPlayer)
			continue;
		if(pPlayer->IsDormant())
			continue;

		BOOL bPlayedSnd = (g_pGameRules->m_iRoundWinStatus) ? TRUE : FALSE;

		if(m_iTimeAdded)
		{
			// TODO Refactor
			pPlayer->SendAddTime(iTimeAdded);

			if(!bPlayedSnd && RANDOM_LONG(0, 2) == 0)
			{
				bPlayedSnd = TRUE;
				UTIL_PlayMP3(pPlayer, (pPlayer->m_iTeam == pev->team) ? "sound/CKF_III/ano/announcer_time_added.mp3" : "sound/CKF_III/ano/announcer_time_added_enemy.mp3");
			}

			if(!bPlayedSnd && RANDOM_LONG(0, 1) == 0)
			{
				bPlayedSnd = TRUE;
				UTIL_PlayMP3(pPlayer, "sound/CKF_III/ano/announcer_time_awarded.mp3");
			}
		}

		if(!bPlayedSnd)
		{
			bPlayedSnd = TRUE;
			if(pev->team != 0)
			{
				UTIL_PlayMP3(pPlayer, (pPlayer->m_iTeam == pev->team) ? "sound/CKF_III/ano/announcer_we_captured_control.mp3" : "sound/CKF_III/ano/announcer_we_lost_control.mp3");
			}
			else
			{
				UTIL_PlayMP3(pPlayer, (pPlayer->m_iTeam == pev->team) ? "sound/CKF_III/ano/announcer_we_captured_center_control.mp3" : "sound/CKF_III/ano/announcer_we_lost_center_control.mp3");
			}
		}

		if(pPlayer->m_iTeam == pev->team)
		{
			UTIL_PlayWAV(pPlayer, "CKF_III/scored.wav");
			if(pPlayer->m_pentControlPoint == edict()) 
			{
				plCapturer[iCapturer] = pPlayer;
				iCapturer ++;

				pPlayer->AddPoints(2, TRUE);
				pPlayer->m_Stats.iCapture ++;
				pPlayer->SendStatsInfo(STATS_CAPTURE);
			}
		}
	}

	g_pGameRules->ObjectNotice(m_iIndex, 0, pev->team, iOldTeam, plCapturer, iCapturer);

	if(iTimeAdded && g_pGameRules->m_iRoundStatus == ROUND_NORMAL)
	{
		// TODO Refactor
		// Add time to all timer for now
		for (int i = 0; i < g_pGameRules->m_RoundTimers.Count(); i++)
		{
			CRoundTimer *pTimer = (CRoundTimer *)CBaseEntity::Instance(g_pGameRules->m_RoundTimers[i]);
			pTimer->AddTime(iTimeAdded);
		}
		g_pGameRules->RTSendState();
	}
}

void CControlPoint::ControlPointCapturing(int iCapTeam)
{
	if(pev->team == 1 || pev->team == 2)
	{
		if(g_pGameRules->CPCountPoints(pev->team) == 1)
		{
			switch(RANDOM_LONG(0,1))
			{
			case 0:UTIL_PlayTeamMP3(pev->team, "sound/CKF_III/ano/announcer_last_flag.mp3");break;
			case 1:UTIL_PlayTeamMP3(pev->team, "sound/CKF_III/ano/announcer_last_flag2.mp3");break;
			}
		}
		else
		{
			switch(RANDOM_LONG(0,2))
			{
			case 0:UTIL_PlayTeamMP3(pev->team, "sound/CKF_III/ano/announcer_control_point_warning.mp3");break;
			case 1:UTIL_PlayTeamMP3(pev->team, "sound/CKF_III/ano/announcer_control_point_warning2.mp3");break;
			case 2:UTIL_PlayTeamMP3(pev->team, "sound/CKF_III/ano/announcer_control_point_warning3.mp3");break;
			}
		}
	}
	else
	{
		switch(RANDOM_LONG(0,1))
		{
		case 0:UTIL_PlayMP3(NULL, "sound/CKF_III/ano/announcer_alert_center_being_captured.mp3");break;
		case 1:UTIL_PlayMP3(NULL, "sound/CKF_III/ano/announcer_alert_center_being_contested.mp3");break;
		}
	}
}

void CControlPoint::ControlPointDefend(int iCapTeam)
{
	int iCapturer = 0;
	CBasePlayer *plCapturer[32];
	//Players event
	for(int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex(i);
		if(!pPlayer || !pPlayer->IsPlayer())
			continue;
		if(pPlayer->IsDormant())
			continue;
		if(pPlayer->m_pentControlPoint == edict() && pPlayer->m_iTeam == 3-iCapTeam) 
		{
			plCapturer[iCapturer] = pPlayer;
			iCapturer ++;
			pPlayer->AddPoints(1, TRUE);
			pPlayer->m_Stats.iDefence ++;
			pPlayer->SendStatsInfo(STATS_DEFENCE);
		}
	}

	g_pGameRules->ObjectNotice(m_iIndex, 1, 3-iCapTeam, pev->team, plCapturer, iCapturer);
}

void CControlPoint::ControlPointTouch(CBaseEntity *pOther)
{
	if (!pOther->IsPlayer())
		return;

	if(!pOther->IsAlive())
		return;

	CBasePlayer *pPlayer = (CBasePlayer *)pOther;

	if(m_bLocked || m_bDisabled)
		return;

	if(g_pGameRules->m_iRoundStatus == ROUND_END)
		return;

	if(!pPlayer->CanCapture())
		return;

	if(!(pPlayer->m_iMapZone & MAPZONE_CONTROLPOINT))
	{
		pPlayer->m_iMapZone |= MAPZONE_CONTROLPOINT;
		pPlayer->m_pentControlPoint = ENT(pev);
	}
}

float GetHarmonicSeries(int n)
{
	static float flSeriesArray[32];
	if(n < 1 || n > 32)
		return 0;
	if(!flSeriesArray[n-1])
	{
		float a = 0;
		for(int i = 1; i <= n; ++i)
		{
			a += 1.0f / i;
		}
		flSeriesArray[n-1] = a;
	}

	return flSeriesArray[n-1];
}

void CControlPoint::ControlPointThink(void)
{
	pev->nextthink = gpGlobals->time + 0.1;

	if(m_bLocked || m_bDisabled)
		return;

	float flCapRate = m_flCaptureRate * 0.1;
	int iRedCaps = 0;
	int iBluCaps = 0;
	float flRedCaps = 0;
	float flBluCaps = 0;
	int iCapTeam = 0;
	int iDefTeam = 0;

	BOOL bIsCapuring = FALSE;
	BOOL bIsDefending = FALSE;
	BOOL bBlocked = FALSE;
	for(int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CBaseEntity *pEnt = UTIL_PlayerByIndex(i);
		if(pEnt && pEnt->IsPlayer() && !pEnt->IsDormant())
		{
			CBasePlayer *pPlayer = (CBasePlayer *)pEnt;
			if(!pPlayer->IsAlive())
				continue;
			if(!(pPlayer->m_iMapZone & MAPZONE_CONTROLPOINT))
				continue;
			if(pPlayer->m_pentControlPoint != edict())
				continue;

			if(pPlayer->m_iTeam == TEAM_RED)
			{
				iRedCaps += (pPlayer->m_iClass == CLASS_SCOUT) ? 2 : 1;
				flRedCaps += pPlayer->GetCapRate();
			}
			else if(pPlayer->m_iTeam == TEAM_BLU)
			{
				iBluCaps += (pPlayer->m_iClass == CLASS_SCOUT) ? 2 : 1;
				flBluCaps += pPlayer->GetCapRate();
			}
		}
	}

	//Both red and blue team have players on this control point, so block it
	if(iRedCaps && iBluCaps)
	{
		bBlocked = TRUE;
	}
	//Is there any player defending this point?
	if(iBluCaps && m_iCapTeam == TEAM_RED)
	{
		bIsDefending = TRUE;
		iDefTeam = TEAM_BLU;
	}
	else if(iRedCaps && m_iCapTeam == TEAM_BLU)
	{
		bIsDefending = TRUE;
		iDefTeam = TEAM_RED;
	}
	//Is there any player capturing this point?
	if(iRedCaps && m_bCanRedCap && pev->team != TEAM_RED)
	{
		bIsCapuring = TRUE;
		iCapTeam = TEAM_RED;
	}
	else if(iBluCaps && m_bCanBluCap && pev->team != TEAM_BLU)
	{
		bIsCapuring = TRUE;
		iCapTeam = TEAM_BLU;
	}

	switch(m_iState)
	{
	case CP_IDLE:
		if(bBlocked)
		{
			m_iState = CP_BLOCKED;
		}
		else if(bIsCapuring)
		{
			m_iCapTeam = iCapTeam;
			m_iState = CP_CAPTURING;
		}
		break;
	case CP_CAPTURING:
		if(bBlocked)
		{
			ControlPointDefend(iCapTeam);
			m_iState = CP_BLOCKED;
		}
		else if(!bIsCapuring)
		{
			m_iState = CP_BACK;
		}
		else if(iCapTeam == m_iCapTeam)
		{
			flCapRate *= GetHarmonicSeries((iCapTeam == 1) ? flRedCaps : flBluCaps);
			m_flProgress = min(m_flProgress + flCapRate, 100);
			if(m_flProgress == 100)
			{
				ControlPointCaptured(m_iCapTeam);
				//return;
			}
		}
		break;
	case CP_BLOCKED:
		if(!bBlocked)
		{
			m_iState = (bIsCapuring && iCapTeam == m_iCapTeam) ? CP_CAPTURING : CP_BACK;
		}
		break;
	case CP_BACK:
		if(bBlocked)
		{
			m_iState = CP_BLOCKED;
		}
		else if(bIsCapuring && iCapTeam == m_iCapTeam)
		{
			m_iState = CP_CAPTURING;
		}
		else
		{
			if(bIsDefending)
				flCapRate *= GetHarmonicSeries((iDefTeam == 1) ? flRedCaps : flBluCaps);
			else
				flCapRate *= 0.5f;
			m_flProgress = max(m_flProgress - flCapRate, 0);
			if(m_flProgress == 0)
			{
				m_iCapTeam = 0;
				m_iState = CP_IDLE;

				g_pGameRules->CheckWinConditions();
			}
		}
		break;
	}
	m_iCapPlayers = max(iRedCaps, iBluCaps);
	m_flCapRate = flCapRate*10.0;
	UpdateHUD();
}

void CControlPoint::UpdateTeam(int iNewTeam)
{
	pev->team = iNewTeam;
	m_flProgress = 0;
	m_iState = CP_IDLE;
	m_iCapTeam = 0;

	if (m_pSubEntity)
		m_pSubEntity->pev->skin = pev->team;
}

void CControlPoint::UpdateHUD(void)
{
	if(m_iState == CP_CAPTURING && m_iClientState != m_iState)//Cap MP3 Announcer
	{
		ControlPointCapturing(m_iCapTeam);
	}

	if(m_iClientState != m_iState || m_iClientCapTeam != m_iCapTeam || m_flClientCapRate != m_flCapRate || m_iClientCapPlayers != m_iCapPlayers || (m_flClientSendState < gpGlobals->time && m_flClientSendState > 0))
	{
		m_iClientState = m_iState;
		m_iClientCapTeam = m_iCapTeam;
		m_flClientCapRate = m_flCapRate;
		m_iClientCapPlayers = m_iCapPlayers;
		
		if(m_iState == CP_CAPTURING || m_iState == CP_BACK)
			m_flClientSendState = gpGlobals->time + 3.0;
		else
			m_flClientSendState = 0;

		g_pGameRules->CPSendState(pev);
	}
}

LINK_ENTITY_TO_CLASS(trigger_roundtimer, CRoundTimer);

void CRoundTimer::Spawn(void)
{
	InitTrigger();
	pev->nextthink = gpGlobals->time + 1.0;
}

void CRoundTimer::PostSpawn(void)
{
	m_iIndex = g_pGameRules->m_RoundTimers.Count() + 1;
	g_pGameRules->m_RoundTimers.AddToTail(edict());
}

void CRoundTimer::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "timeoutaction"))
	{
		m_iTimeoutAction = atoi(pkvd->szValue);

		m_iTimeoutAction = min(max(m_iTimeoutAction, WINSTATUS_NONE), WINSTATUS_DRAW);

		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "trigontimeout"))
	{
		m_iTrigOnTimeout = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "hudpos"))
	{
		m_iHUDPosition = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "totaltime"))
	{
		m_flOriginTotalTime = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "announcetime"))
	{
		m_bAnnounceTime = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "locked"))
	{
		m_bOriginLocked = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "disabled"))
	{
		m_bOriginDisabled = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

void CRoundTimer::Restart(void)
{
	m_bTiming = FALSE;
	m_bOvertime = FALSE;
	m_flTime = m_flOriginTotalTime;
	m_flTotalTime = m_flOriginTotalTime;
	m_flEndTime = 0.0f;
	m_bLocked = m_bOriginLocked;
	m_bDisabled = m_bOriginDisabled;
	m_iLastAnnounceTime = -1;
}

void CRoundTimer::RoundTimerUpdate(void)
{
	if (m_bLocked || m_bDisabled)
		RoundTimerPause();
	else
		RoundTimerResume();
}

void CRoundTimer::RoundTimerPause(void)
{
	if (!m_bTiming)
		return;
	m_flTime = max(TimeRemaining(), 0.0f);
	m_flEndTime = 0.0f;
	m_bTiming = FALSE;
}

void CRoundTimer::RoundTimerResume(void)
{
	if (m_bTiming)
		return;
	m_flEndTime = gpGlobals->time + m_flTime;
	m_bTiming = TRUE;
}

void CRoundTimer::UpdateTime(float time)
{
	m_flTime = fmaxf(time, 0.0f);
	m_flTotalTime = m_flTime;
	m_flEndTime = m_bTiming ? gpGlobals->time + m_flTime : 0.0f;
	if (TimeRemaining() > 0)
		m_bOvertime = FALSE;
}

void CRoundTimer::AddTime(float time)
{
	m_flTime = fmaxf(fmaxf(TimeRemaining(), 0.0f) + time, 0.0f);
	m_flTotalTime = m_flTime;
	m_flEndTime = m_bTiming ? gpGlobals->time + m_flTime : 0.0f;
	if (TimeRemaining() > 0)
		m_bOvertime = FALSE;
}

float CRoundTimer::TimeRemaining(void)
{
	return m_bTiming ? m_flEndTime - gpGlobals->time : m_flTime;
}

float CRoundTimer::TimeElapsed(void)
{
	return TimeTotal() - TimeRemaining();
}

float CRoundTimer::TimeTotal(void)
{
	return m_flTotalTime;
}

class CNoBuildZone : public CBaseTrigger
{
public:
	void Spawn(void);
};

LINK_ENTITY_TO_CLASS(func_nobuildzone, CNoBuildZone);

void CNoBuildZone::Spawn(void)
{
	InitTrigger();

	if (pev->team > TEAM_CT || pev->team < TEAM_UNASSIGNED)
	{
		ALERT(at_console, "Bad team number (%i) in %s\n", pev->team, STRING(pev->classname));
		pev->team = 0;
	}
	g_pGameRules->m_NoBuildZone.AddToTail(edict());
}

//cs16

class CWeather : public CBaseTrigger
{
public:
	void Spawn(void) { InitTrigger(); }
};

LINK_ENTITY_TO_CLASS(env_snow, CWeather);
LINK_ENTITY_TO_CLASS(func_snow, CWeather);
LINK_ENTITY_TO_CLASS(env_rain, CWeather);
LINK_ENTITY_TO_CLASS(func_rain, CWeather);

void CClientFog::Spawn(void)
{
	pev->movetype = MOVETYPE_NOCLIP;
	pev->solid = SOLID_NOT;
	pev->renderamt = 0;
	pev->rendermode = kRenderNormal;
}

void CClientFog::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "density"))
	{
		m_fDensity = atof(pkvd->szValue);

		if (m_fDensity < 0 || m_fDensity > 0.01)
			m_fDensity = 0;

		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

LINK_ENTITY_TO_CLASS(env_fog, CClientFog);