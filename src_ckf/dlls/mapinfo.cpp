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

		m_iSetupCondition = min(max(m_iSetupCondition, ROUND_NORMAL), ROUND_WAIT);

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
	else if (FStrEq(pkvd->szKeyName, "endtime"))
	{
		m_iEndTime = max(atoi(pkvd->szValue), 0);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "reddominatedaction"))
	{
		m_iRedDominatedAction = atoi(pkvd->szValue);

		m_iRedDominatedAction = min(max(m_iRedDominatedAction, WINSTATUS_NONE), WINSTATUS_DRAW);

		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "bludominatedaction"))
	{
		m_iBluDominatedAction = atoi(pkvd->szValue);

		m_iBluDominatedAction = min(max(m_iBluDominatedAction, WINSTATUS_NONE), WINSTATUS_DRAW);

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
	else if (FStrEq(pkvd->szKeyName, "canredcapop"))
	{
		op = atoi(pkvd->szValue);
		m_opCanRedCapOp = op >= SKIP && op <= RESET ? CONTROL_OP(op) : SKIP;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "canredcaparg"))
	{
		m_bCanRedCapArg = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "canblucapop"))
	{
		op = atoi(pkvd->szValue);
		m_opCanBluCapOp = op >= SKIP && op <= RESET ? CONTROL_OP(op) : SKIP;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "canblucaparg"))
	{
		m_bCanBluCapArg = atoi(pkvd->szValue);
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

		switch (m_opCanRedCapOp)
		{
		case SKIP:
			break;
		case UPDATE:
			pPoint->m_bCanRedCap = m_bCanRedCapArg;
			break;
		case TOGGLE:
			pPoint->m_bCanRedCap = !pPoint->m_bCanRedCap;
			break;
		case RESET:
			pPoint->m_bCanRedCap = pPoint->m_bOriginCanRedCap;
			break;
		}

		switch (m_opCanBluCapOp)
		{
		case SKIP:
			break;
		case UPDATE:
			pPoint->m_bCanBluCap = m_bCanBluCapArg;
			break;
		case TOGGLE:
			pPoint->m_bCanBluCap = !pPoint->m_bCanBluCap;
			break;
		case RESET:
			pPoint->m_bCanBluCap = pPoint->m_bOriginCanBluCap;
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

		if (m_opLockedOp || m_opDisabledOp || m_opCanRedCapOp || m_bCanBluCapArg || m_opTeamOp)
			g_pGameRules->CPSendState(pPoint->pev);
	}
}

LINK_ENTITY_TO_CLASS(trigger_roundterminator, CRoundTerminator);

void CRoundTerminator::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "winstatus"))
	{
		m_iWinStatus = atoi(pkvd->szValue);
		m_iWinStatus = max(min(m_iWinStatus, WINSTATUS_DRAW), WINSTATUS_NONE);
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