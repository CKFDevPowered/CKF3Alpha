/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
/*

  Last Modifed 4 May 2004 By Andrew Hamilton (AJH)
  :- Added support for acceleration of doors etc

===== subs.cpp ========================================================

  frequently used global functions

*/

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "nodes.h"
#include "doors.h"
#include "player.h"

#define ACCELTIMEINCREMENT 0.1 //AJH for acceleration/deceleration time steps

extern CGraph WorldGraph;

extern BOOL FEntIsVisible(entvars_t* pev, entvars_t* pevTarget);

extern DLL_GLOBAL int g_iSkillLevel;

void CBaseEntity::UpdateOnRemove(void)
{
	if (FBitSet(pev->flags, FL_GRAPHED))
		for (int i = 0; i < WorldGraph.m_cLinks; i++)
			if (WorldGraph.m_pLinkPool[i].m_pLinkEnt == pev)
				WorldGraph.m_pLinkPool[i].m_pLinkEnt = NULL;

	if (pev->globalname)
		gGlobalState.EntitySetState(pev->globalname, GLOBAL_DEAD);
}

void CBaseEntity::SUB_Remove(void)
{
	UpdateOnRemove();

	if (pev->health > 0)
	{
		pev->health = 0;
		ALERT(at_aiconsole, "SUB_Remove called on entity with health > 0\n");
	}

	REMOVE_ENTITY(ENT(pev));
}

void CBaseEntity::SUB_DoNothing(void)
{
}

void CBaseDelay :: KeyValue( KeyValueData *pkvd )
{
	if (FStrEq(pkvd->szKeyName, "delay"))
	{
		m_flDelay = atof( pkvd->szValue );
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "killtarget"))
	{
		m_iszKillTarget = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
	{
		CBaseEntity::KeyValue( pkvd );
	}
}

void CBaseEntity::SUB_UseTargets(CBaseEntity *pActivator, USE_TYPE useType, float value)
{
	if (!FStringNull(pev->target))
		FireTargets(STRING(pev->target), pActivator, this, useType, value);
}

void FireTargets(const char *targetName, CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	edict_t *pentTarget = NULL;

	if (!targetName)
		return;

	ALERT(at_aiconsole, "Firing: (%s)\n", targetName);

	while (1)
	{
		pentTarget = FIND_ENTITY_BY_TARGETNAME(pentTarget, targetName);

		if (FNullEnt(pentTarget))
			break;

		CBaseEntity *pTarget = CBaseEntity::Instance(pentTarget);

		if (pTarget && !(pTarget->pev->flags & FL_KILLME))
		{
			ALERT(at_aiconsole, "Found: %s, firing (%s)\n", STRING(pTarget->pev->classname), targetName);
			pTarget->Use(pActivator, pCaller, useType, value);
		}
	}
}

LINK_ENTITY_TO_CLASS( DelayedUse, CBaseDelay );

void CBaseDelay::SUB_UseTargets(CBaseEntity *pActivator, USE_TYPE useType, float value)
{
	if (FStringNull(pev->target) && !m_iszKillTarget)
		return;

	if (m_flDelay != 0)
	{
		CBaseDelay *pTemp = GetClassPtr((CBaseDelay *)NULL);
		pTemp->pev->classname = MAKE_STRING("DelayedUse");
		pTemp->pev->nextthink = gpGlobals->time + m_flDelay;
		pTemp->SetThink(&CBaseDelay::DelayThink);
		pTemp->pev->button = (int)useType;
		pTemp->m_iszKillTarget = m_iszKillTarget;
		pTemp->m_flDelay = 0;
		pTemp->pev->target = pev->target;

		if (pActivator && pActivator->IsPlayer())
			pTemp->pev->owner = pActivator->edict();
		else
			pTemp->pev->owner = NULL;

		return;
	}

	if (m_iszKillTarget)
	{
		ALERT(at_aiconsole, "KillTarget: %s\n", STRING(m_iszKillTarget));
		edict_t *pentKillTarget = FIND_ENTITY_BY_TARGETNAME(NULL, STRING(m_iszKillTarget));

		while (!FNullEnt(pentKillTarget))
		{
			UTIL_Remove(CBaseEntity::Instance(pentKillTarget));
			ALERT(at_aiconsole, "killing %s\n", STRING(pentKillTarget->v.classname));
			pentKillTarget = FIND_ENTITY_BY_TARGETNAME(pentKillTarget, STRING(m_iszKillTarget));
		}
	}

	if (!FStringNull(pev->target))
		FireTargets(STRING(pev->target), pActivator, this, useType, value);
}

void SetMovedir(entvars_t *pev)
{
	if (pev->angles == Vector(0, -1, 0))
	{
		pev->movedir = Vector(0, 0, 1);
	}
	else if (pev->angles == Vector(0, -2, 0))
	{
		pev->movedir = Vector(0, 0, -1);
	}
	else
	{
		UTIL_MakeVectors(pev->angles);
		pev->movedir = gpGlobals->v_forward;
	}

	pev->angles = g_vecZero;
}

Vector GetMovedir( Vector vecAngles )
{
	if (vecAngles == Vector(0, -1, 0))
	{
		return Vector(0, 0, 1);
	}
	else if (vecAngles == Vector(0, -2, 0))
	{
		return Vector(0, 0, -1);
	}
	else
	{
		UTIL_MakeVectors(vecAngles);
		return gpGlobals->v_forward;
	}
}

void CBaseDelay::DelayThink(void)
{
	CBaseEntity *pActivator = NULL;

	if (pev->owner)
		pActivator = CBaseEntity::Instance(pev->owner);

	SUB_UseTargets(pActivator, (USE_TYPE)pev->button, 0);
	REMOVE_ENTITY(ENT(pev));
}

void CBaseToggle::KeyValue( KeyValueData *pkvd )
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
	else if (FStrEq(pkvd->szKeyName, "master"))
	{
		m_sMaster = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "distance"))
	{
		m_flMoveDistance = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseDelay::KeyValue( pkvd );
}

void CBaseToggle::LinearMove(Vector vecDest, float flSpeed)
{
	m_vecFinalDest = vecDest;

	if (vecDest == pev->origin)
	{
		LinearMoveDone();
		return;
	}

	Vector vecDestDelta = vecDest - pev->origin;
	float flTravelTime = vecDestDelta.Length() / flSpeed;
	pev->nextthink = pev->ltime + flTravelTime;
	SetThink(&CBaseToggle::LinearMoveDone);
	pev->velocity = vecDestDelta / flTravelTime;
}

void CBaseToggle::LinearMoveDone(void)
{
	UTIL_SetOrigin(pev, m_vecFinalDest);
	pev->velocity = g_vecZero;
	pev->nextthink = -1;

	if (m_pfnCallWhenMoveDone)
		(this->*m_pfnCallWhenMoveDone)();
}

BOOL CBaseToggle::IsLockedByMaster(void)
{
	if (m_sMaster && !UTIL_IsMasterTriggered(m_sMaster, m_hActivator))
		return TRUE;
	else
		return FALSE;
}

void CBaseToggle::AngularMove(Vector vecDestAngle, float flSpeed)
{
	m_vecFinalAngle = vecDestAngle;

	if (vecDestAngle == pev->angles)
	{
		AngularMoveDone();
		return;
	}

	Vector vecDestDelta = vecDestAngle - pev->angles;
	float flTravelTime = vecDestDelta.Length() / flSpeed;
	pev->nextthink = pev->ltime + flTravelTime;
	SetThink(&CBaseToggle::AngularMoveDone);
	pev->avelocity = vecDestDelta / flTravelTime;
}

void CBaseToggle::AngularMoveDone(void)
{
	pev->angles = m_vecFinalAngle;
	pev->avelocity = g_vecZero;
	pev->nextthink = -1;

	if (m_pfnCallWhenMoveDone)
		(this->*m_pfnCallWhenMoveDone)();
}

float CBaseToggle::AxisValue(int flags, const Vector &angles)
{
	if (FBitSet(flags, SF_DOOR_ROTATE_Z))
		return angles.z;

	if (FBitSet(flags, SF_DOOR_ROTATE_X))
		return angles.x;

	return angles.y;
}

void CBaseToggle::AxisDir(entvars_t *pev)
{
	if (FBitSet(pev->spawnflags, SF_DOOR_ROTATE_Z))
		pev->movedir = Vector(0, 0, 1);
	else if (FBitSet(pev->spawnflags, SF_DOOR_ROTATE_X))
		pev->movedir = Vector(1, 0, 0);
	else
		pev->movedir = Vector(0, 1, 0);
}

float CBaseToggle::AxisDelta(int flags, const Vector &angle1, const Vector &angle2)
{
	if (FBitSet(flags, SF_DOOR_ROTATE_Z))
		return angle1.z - angle2.z;

	if (FBitSet(flags, SF_DOOR_ROTATE_X))
		return angle1.x - angle2.x;

	return angle1.y - angle2.y;
}

BOOL FEntIsVisible(entvars_t *pev, entvars_t *pevTarget)
{
	Vector vecSpot1 = pev->origin + pev->view_ofs;
	Vector vecSpot2 = pevTarget->origin + pevTarget->view_ofs;

	TraceResult tr;
	UTIL_TraceLine(vecSpot1, vecSpot2, ignore_monsters, ENT(pev), &tr);

	if (tr.fInOpen && tr.fInWater)
		return FALSE;

	if (tr.flFraction == 1)
		return TRUE;

	return FALSE;
}