#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"
#include "game.h"
#include "pm_shared.h"

extern int gmsgCrosshair;
extern int gmsgCurWeapon;
extern int gmsgStatusIcon;
extern int gmsgSetFOV;

int GetForceCamera(void)
{
	int iForceCamera;

	if (!fadetoblack.value)
	{
		iForceCamera = (int)CVAR_GET_FLOAT("mp_forcechasecam");

		if (iForceCamera == FORCECAMERA_SPECTATE_ANYONE)
			iForceCamera = (int)CVAR_GET_FLOAT("mp_forcecamera");
	}
	else
		iForceCamera = FORCECAMERA_ONLY_FRIST_PERSON;

	return iForceCamera;
}

CBaseEntity *CBasePlayer::Observer_IsValidTarget(int iTarget, BOOL bOnlyTeam)
{
	if (iTarget > gpGlobals->maxClients || iTarget < 1)
		return NULL;

	CBaseEntity *pEnt = (CBaseEntity *)UTIL_PlayerByIndex(iTarget);

	if (!pEnt || pEnt == this || pEnt->m_bDisconnect || ((CBasePlayer *)pEnt)->IsObserver() || (pEnt->pev->effects & EF_NODRAW) || (bOnlyTeam && ((CBasePlayer *)pEnt)->m_iTeam != m_iTeam))
		return NULL;

	return pEnt;
}

void CBasePlayer::Observer_FindNextPlayer(BOOL bReverse)
{
	int iStart = m_hObserverTarget ? ENTINDEX(m_hObserverTarget->edict()) : ENTINDEX(edict());
	int iCurrent = iStart;
	m_hObserverTarget = NULL;
	int iDir = bReverse ? -1 : 1;
	BOOL bOnlyTeam = FALSE;
	int iForceCamera = GetForceCamera();

	if (iForceCamera != FORCECAMERA_SPECTATE_ANYONE && m_iTeam != TEAM_SPECTATOR)
		bOnlyTeam = TRUE;

	do
	{
		iCurrent += iDir;

		if (iCurrent > gpGlobals->maxClients)
			iCurrent = 1;
		else if (iCurrent < 1)
			iCurrent = gpGlobals->maxClients;

		m_hObserverTarget = Observer_IsValidTarget(iCurrent, bOnlyTeam);

		if (m_hObserverTarget)
			break;
	}
	while (iCurrent != iStart);

	if (m_hObserverTarget)
	{
		UTIL_SetOrigin(pev, m_hObserverTarget->pev->origin);

		if (pev->iuser1 != OBS_ROAMING)
			pev->iuser2 = ENTINDEX(m_hObserverTarget->edict());

		if (pev->iuser1 == OBS_IN_EYE)
			SetProgressBarTime(0);
	}
}

void CBasePlayer::Observer_HandleButtons(void)
{
	if (m_flNextObserverInput > gpGlobals->time)
		return;

	if (m_afButtonPressed & IN_JUMP)
	{
		if (pev->iuser1 == OBS_CHASE_LOCKED)
			Observer_SetMode(OBS_CHASE_FREE);
		else if (pev->iuser1 == OBS_CHASE_FREE)
			Observer_SetMode(OBS_IN_EYE);
		else if (pev->iuser1 == OBS_IN_EYE)
			Observer_SetMode(OBS_ROAMING);
		else if (pev->iuser1 == OBS_ROAMING)
			Observer_SetMode(OBS_MAP_FREE);
		else if (pev->iuser1 == OBS_MAP_FREE)
			Observer_SetMode(OBS_MAP_CHASE);
		else
			Observer_SetMode(OBS_CHASE_FREE);

		m_flNextObserverInput = gpGlobals->time + 0.2;
	}

	if (m_afButtonPressed & IN_ATTACK)
	{
		Observer_FindNextPlayer(FALSE);
		m_flNextObserverInput = gpGlobals->time + 0.2;
	}

	if (m_afButtonPressed & IN_ATTACK2)
	{
		Observer_FindNextPlayer(TRUE);
		m_flNextObserverInput = gpGlobals->time + 0.2;
	}
}

void CBasePlayer::Observer_CheckTarget(void)
{
	if (pev->iuser1 == OBS_ROAMING)
		return;

	if (m_hObserverTarget == NULL)
		Observer_FindNextPlayer(FALSE);

	if (m_hObserverTarget)
	{
		int iTarget = ENTINDEX(ENT(m_hObserverTarget->pev));
		CBasePlayer *pTarget = (CBasePlayer *)((CBaseEntity *)m_hObserverTarget);

		if (!pTarget || pTarget->pev->deadflag == DEAD_RESPAWNABLE || (pTarget->pev->effects & EF_NODRAW) || (pTarget->pev->deadflag == DEAD_DEAD && gpGlobals->time > pTarget->m_fDeadTime + 2))
			Observer_FindNextPlayer(FALSE);
	}
	else
	{
		int iMode = pev->iuser1;
		Observer_SetMode(OBS_ROAMING);
		m_iObserverMode = iMode;
	}
}

void CBasePlayer::Observer_CheckProperties(void)
{
	if (pev->iuser1 == OBS_IN_EYE && m_hObserverTarget != NULL)
	{
		int iTarget = ENTINDEX(ENT(m_hObserverTarget->pev));
		CBasePlayer *pTarget = (CBasePlayer *)((CBaseEntity *)m_hObserverTarget);

		if (!pTarget)
			return;

		int iWeaponId = pTarget->m_pActiveItem ? pTarget->m_pActiveItem->m_iId : 0;

		if (m_iFOV != pTarget->m_iFOV || m_iObserverWeaponId != iWeaponId)
		{
			m_iClientFOV = m_iFOV = pTarget->m_iFOV;

			MESSAGE_BEGIN(MSG_ONE, gmsgSetFOV, NULL, pev);
			WRITE_BYTE(m_iFOV);
			MESSAGE_END();

			m_iObserverWeaponId = iWeaponId;

			MESSAGE_BEGIN(MSG_ONE, gmsgCurWeapon, NULL, pev);
			WRITE_BYTE(1);
			WRITE_BYTE(m_iObserverWeaponId);
			WRITE_BYTE(0);
			MESSAGE_END();
		}

		int iC4State = STATUSICON_HIDE;

/*		if (pTarget->m_bIsBombGuy)
		{
			if (pTarget->m_iClientMapZone & MAPZONE_BOMB)
				iC4State = STATUSICON_FLASH;
			else
				iC4State = STATUSICON_SHOW;
		}*/

		if (m_iObserverC4State != iC4State)
		{
			m_iObserverC4State = iC4State;

			if (iC4State)
			{
				MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, pev);
				WRITE_BYTE(m_iObserverC4State);
				WRITE_STRING("c4");
				WRITE_BYTE(0);
				WRITE_BYTE(160);
				WRITE_BYTE(0);
				MESSAGE_END();
			}
			else
			{
				MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, pev);
				WRITE_BYTE(STATUSICON_HIDE);
				WRITE_STRING("c4");
				MESSAGE_END();
			}
		}

		if (m_bObserverHasDefuseKit != pTarget->m_bHasDefuseKit)
		{
			m_bObserverHasDefuseKit = pTarget->m_bHasDefuseKit;

			if (pTarget->m_bHasDefuseKit)
			{
				MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, pev);
				WRITE_BYTE(STATUSICON_SHOW);
				WRITE_STRING("defuser");
				WRITE_BYTE(0);
				WRITE_BYTE(160);
				WRITE_BYTE(0);
				MESSAGE_END();
			}
			else
			{
				MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, pev);
				WRITE_BYTE(STATUSICON_HIDE);
				WRITE_STRING("defuser");
				MESSAGE_END();
			}
		}

		return;
	}

	m_iFOV = 90;

	if (m_iObserverWeaponId)
	{
		m_iObserverWeaponId = 0;

		MESSAGE_BEGIN(MSG_ONE, gmsgCurWeapon, NULL, pev);
		WRITE_BYTE(1);
		WRITE_BYTE(m_iObserverWeaponId);
		WRITE_BYTE(0);
		MESSAGE_END();
	}

	if (m_iObserverC4State)
	{
		m_iObserverC4State = 0;

		MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, pev);
		WRITE_BYTE(0);
		WRITE_STRING("c4");
		MESSAGE_END();
	}

	if (m_bObserverHasDefuseKit)
	{
		m_bObserverHasDefuseKit = FALSE;

		MESSAGE_BEGIN(MSG_ONE, gmsgStatusIcon, NULL, pev);
		WRITE_BYTE(0);
		WRITE_STRING("defuser");
		MESSAGE_END();
	}
}

void CBasePlayer::Observer_SetMode(int iMode)
{
	if (iMode == pev->iuser1)
		return;

	if (iMode < OBS_CHASE_LOCKED || iMode > OBS_MAP_CHASE)
		iMode = OBS_IN_EYE;

	int iForceCamera = GetForceCamera();

	if (m_iTeam != TEAM_SPECTATOR)
	{
		if (iForceCamera == FORCECAMERA_SPECTATE_ONLY_TEAM)
		{
			if (iMode == OBS_ROAMING)
				iMode = OBS_MAP_FREE;
		}
		else if (iForceCamera == FORCECAMERA_ONLY_FRIST_PERSON)
			iMode = OBS_IN_EYE;
	}

	if (m_hObserverTarget)
	{
		CBasePlayer *pTarget = (CBasePlayer *)((CBaseEntity *)m_hObserverTarget);

		if (pTarget == this || !pTarget || pTarget->m_bDisconnect || pTarget->IsObserver() || (pTarget->pev->effects & EF_NODRAW) || (iForceCamera != FORCECAMERA_SPECTATE_ANYONE && pTarget->m_iTeam == m_iTeam))
			m_hObserverTarget = NULL;
	}

	pev->iuser1 = iMode;

	if (iMode != OBS_ROAMING)
	{
		if (m_hObserverTarget == NULL)
		{
			Observer_FindNextPlayer(FALSE);

			if (m_hObserverTarget == NULL)
			{
				ClientPrint(pev, HUD_PRINTCENTER, "#Spec_NoTarget");
				pev->iuser1 = OBS_ROAMING;
			}
		}
	}

	if (pev->iuser1 != OBS_ROAMING)// && m_hObserverTarget)
		pev->iuser2 = ENTINDEX(ENT(m_hObserverTarget->pev));
	else
		pev->iuser2 = 0;

	pev->iuser3 = 0;

	if (m_hObserverTarget)
		UTIL_SetOrigin(pev, m_hObserverTarget->pev->origin);

	MESSAGE_BEGIN(MSG_ONE, gmsgCrosshair, NULL, pev);
	WRITE_BYTE((iMode == OBS_ROAMING) ? 1 : 0);
	MESSAGE_END();

	SetProgressBarTime(0);

	char text[16];
	sprintf(text, "#Spec_Mode%i", pev->iuser1);
	ClientPrint(pev, HUD_PRINTCENTER, text);
	m_iObserverMode = iMode;
}