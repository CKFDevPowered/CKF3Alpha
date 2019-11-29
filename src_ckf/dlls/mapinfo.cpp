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
	else if (FStrEq(pkvd->szKeyName, "reddominatedaction"))
	{
		m_iRedDominatedAction = atoi(pkvd->szValue);

		if (m_iRedDominatedAction > END_SUDDEN_DEATH)
			m_iRedDominatedAction = END_SUDDEN_DEATH;
		else if (m_iRedDominatedAction < END_NOTHING)
			m_iRedDominatedAction = END_NOTHING;

		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "bludominatedaction"))
	{
		m_iBluDominatedAction = atoi(pkvd->szValue);

		if (m_iBluDominatedAction > END_SUDDEN_DEATH)
			m_iBluDominatedAction = END_SUDDEN_DEATH;
		else if (m_iBluDominatedAction < END_NOTHING)
			m_iBluDominatedAction = END_NOTHING;

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
	else
		CPointEntity::KeyValue(pkvd);
}

void CMapInfo::Spawn(void)
{
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;
	pev->effects |= EF_NODRAW;
}

LINK_ENTITY_TO_CLASS(trigger_cpcontrols, CCPControls);

void CCPControls::KeyValue(KeyValueData *pkvd)
{
	int op;

	if (FStrEq(pkvd->szKeyName, "lockedop"))
	{
		op = atoi(pkvd->szValue);
		m_opLockedOp = op >= SKIP && op <= RESET ? CONTROL_OP(op) : SKIP;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "lockedarg"))
	{
		m_bLockedArg = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "disabledop"))
	{
		op = atoi(pkvd->szValue);
		m_opDisabledOp = op >= SKIP && op <= RESET ? CONTROL_OP(op) : SKIP;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "disabledarg"))
	{
		m_bDisabledArg = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "teamop"))
	{
		op = atoi(pkvd->szValue);
		m_opTeamOp = op >= SKIP && op <= RESET ? CONTROL_OP(op) : SKIP;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "teamarg"))
	{
		m_iTeamArg = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CPointEntity::KeyValue(pkvd);
}

void CCPControls::Spawn(void)
{
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_NOT;
	pev->effects |= EF_NODRAW;
}

void CCPControls::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	CBaseEntity *pEntity = NULL;
	CControlPoint *pPoint = NULL;

	if(!pev->target) return;

	while ((pEntity = UTIL_FindEntityByTargetname(pEntity, STRING(pev->target))) != NULL)
	{
		if(!pEntity) continue;
		if(pEntity->Classify() != CLASS_CONTROLPOINT) continue;
		pPoint = dynamic_cast<CControlPoint *>(pEntity);
		if (!pPoint) continue;

		switch (m_opLockedOp)
		{
		case SKIP:
			break;
		case UPDATE:
			pPoint->m_bLocked = m_bLockedArg;
			break;
		case TOGGLE:
			pPoint->m_bLocked = !pPoint->m_bLocked;
			break;
		case RESET:
			pPoint->m_bLocked = pPoint->m_bOriginLocked;
			break;
		}

		switch (m_opDisabledOp)
		{
		case SKIP:
			break;
		case UPDATE:
			pPoint->m_bDisabled = m_bDisabledArg;
			break;
		case TOGGLE:
			pPoint->m_bDisabled = !pPoint->m_bDisabled;
			break;
		case RESET:
			pPoint->m_bDisabled = pPoint->m_bOriginDisabled;
			break;
		}

		switch (m_opTeamOp)
		{
		case SKIP:
			break;
		case UPDATE:
			pPoint->UpdateTeam(m_iTeamArg);
			break;
		case TOGGLE:
			if (pPoint->pev->team == TEAM_RED)
				pPoint->UpdateTeam(TEAM_BLU);
			if (pPoint->pev->team == TEAM_BLU)
				pPoint->UpdateTeam(TEAM_RED);
			break;
		case RESET:
			pPoint->UpdateTeam(pPoint->m_iOriginTeam);
			break;
		}

		if (m_opLockedOp || m_opDisabledOp || m_opTeamOp)
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
	else
		CPointEntity::KeyValue(pkvd);
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