#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "mapinfo.h"
#include "gamerules.h"
#include "trigger.h"

LINK_ENTITY_TO_CLASS(info_map_settings, CMapInfo);

void CMapInfo::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "setupcondition"))
	{
		m_iSetupCondition = atoi(pkvd->szValue);

		if(m_iSetupCondition > ROUND_WAIT)
			m_iSetupCondition = ROUND_WAIT;
		else if(m_iSetupCondition < ROUND_NORMAL)
			m_iSetupCondition = ROUND_NORMAL;

		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "endaction"))
	{
		m_iEndAction = atoi(pkvd->szValue);

		if(m_iEndAction > END_SUDDEN_DEATH)
			m_iEndAction = END_SUDDEN_DEATH;
		else if(m_iEndAction < END_NOTHING)
			m_iEndAction = END_NOTHING;

		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "waittime"))
	{
		m_iWaitTime = max(atoi(pkvd->szValue), 0);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "freezetime"))
	{
		m_iFreezeTime = max(atoi(pkvd->szValue), 0);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "setuptime"))
	{
		m_iSetupTime = max(atoi(pkvd->szValue), 0);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "roundtime"))
	{
		m_iRoundTime = max(atoi(pkvd->szValue), 0);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "endtime"))
	{
		m_iEndTime = max(atoi(pkvd->szValue), 0);
		pkvd->fHandled = TRUE;
	}
}

void CMapInfo::Spawn(void)
{
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;
	pev->effects |= EF_NODRAW;
}

LINK_ENTITY_TO_CLASS(trigger_cplocker, CCPLocker);

void CCPLocker::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "lockstate"))
	{
		m_iLockState = atoi(pkvd->szValue);
		m_iLockState = max(min(m_iLockState, 2), 0);
		pkvd->fHandled = TRUE;
	}
}

void CCPLocker::Spawn(void)
{
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;
	pev->effects |= EF_NODRAW;
}

void CCPLocker::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	CBaseEntity *pEntity = NULL;
	CControlPoint *pPoint = NULL;

	if(!pev->target) return;

	while ((pEntity = UTIL_FindEntityByTargetname(pEntity, STRING(pev->target))) != NULL)
	{
		if(!pEntity) continue;
		if(pEntity->Classify() != CLASS_CONTROLPOINT) continue;
		pPoint = (CControlPoint *)pEntity;

		if(m_iLockState == 1)
			pPoint->m_iState = CP_LOCKED;
		else if(m_iLockState >= 2)
		{
			if(pPoint->m_iState == CP_LOCKED)
				pPoint->m_iState = 0;
		}
		else
		{
			if(pPoint->m_iState == CP_LOCKED)
				pPoint->m_iState = 0;
			else
				pPoint->m_iState = CP_LOCKED;
		}
		g_pGameRules->CPSendState(pPoint->pev);
	}
}

LINK_ENTITY_TO_CLASS(trigger_roundterminator, CRoundTerminator);

void CRoundTerminator::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "winstatus"))
	{
		m_iWinStatus = atoi(pkvd->szValue);
		m_iWinStatus = max(min(m_iWinStatus, 2), 0);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "endtime"))
	{
		m_iEndTime = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
}

void CRoundTerminator::Spawn(void)
{
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;
	pev->effects |= EF_NODRAW;
}

void CRoundTerminator::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	g_pGameRules->TerminateRound((m_iEndTime >= 0) ? m_iEndTime : g_pGameRules->m_iEndTime, m_iWinStatus);
}