#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "weapons.h"
#include "nodes.h"
#include "soundent.h"
#include "decals.h"
#include "player.h"
#include "gamerules.h"
#include "hltv.h"

LINK_ENTITY_TO_CLASS(grenade, CGrenade);

int CGrenade::FShouldCollide(CBaseEntity *pHit)
{
	if(pHit->IsBSPModel() && pHit->pev->team != 0)
	{
		if(pHit->pev->team == m_iTeam)
			return 0;
		if(pHit->pev->team == 3)
			return 0;
		if(g_pGameRules->m_iRoundStatus == ROUND_END)
			return 0;
		return 1;
	}
	if(pHit->IsPlayer())
	{
		CBasePlayer *pPlayer = (CBasePlayer *)pHit;
		if(pPlayer->m_iTeam == m_iTeam && pev->owner != pPlayer->edict())
			return 0;
		return 1;
	}
	if(pHit->Classify() == CLASS_BUILDABLE)
	{
		CBaseBuildable *pBuild = (CBaseBuildable *)pHit;
		if(!pBuild->m_pPlayer)
			return 0;
		if(pBuild->m_iTeam == m_iTeam)
			return 0;
		return 1;
	}
	if(pHit->Classify() == CLASS_PROJECTILE)
	{
		//projectile do not touch with each other
		return 0;
	}
	return 1;
}

int CGrenade::TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType)
{
	if (pev->takedamage <= DAMAGE_YES)
		return 0;

	CBaseEntity *pInflictor = CBaseEntity::Instance(pevInflictor);

	if(pInflictor)
	{
		//projectiles never get damaged by another projectiles.
		if(pInflictor->Classify() == CLASS_PROJECTILE)
			return 0;
	}

	pev->health -= flDamage;

	if (pev->health <= 0)
	{
		Killed(pevAttacker, GIB_NORMAL);
		return 0;
	}

	return 1;
}

void CGrenade::CKFExplode(TraceResult *pTrace, int bitsDamageType)
{
	pev->solid = SOLID_NOT;
	pev->takedamage = DAMAGE_NO;

	entvars_t *pevOwner = NULL;
	if(!m_pDeflecter)
	{
		if (pev->owner)
			pevOwner = VARS(pev->owner);
	}
	else
	{
		pevOwner = m_pDeflecter->pev;
		pev->owner = ENT(pevOwner);
		m_iTeam = m_pDeflecter->m_iTeam;
	}

	entvars_t *pevDirectHit = NULL;
	if (pev->enemy)
		pevDirectHit = VARS(pev->enemy);

	m_iTrace = *pTrace;

	CKFRadiusDamage(pTrace->vecEndPos, pevDirectHit, pev, pevOwner, m_fDmg, m_fDmgRadius, 0, bitsDamageType, m_fForce, m_fForceRadius, m_iCrit);

	pev->owner = NULL;
	pev->nextthink = (pev->velocity.Length() > 600) ? gpGlobals->time + 0.125 : gpGlobals->time + 0.05;
	pev->velocity = g_vecZero;
	SetThink(&CGrenade::CKFDisappear);
}

void CGrenade::CKFDisappear(void)
{
	pev->effects |= EF_NODRAW;
	pev->model = 0;
	pev->movetype = MOVETYPE_NONE;

	MESSAGE_BEGIN(MSG_BROADCAST, gmsgDrawFX);
	WRITE_BYTE(FX_KILLTRAIL);
	WRITE_SHORT(ENTINDEX(edict()));
	MESSAGE_END();

	if (m_iTrace.flFraction < 1)
		pev->origin = m_iTrace.vecEndPos + m_iTrace.vecPlaneNormal*5;

	/*if(m_iTrace.flFraction < 1)
	{
		MESSAGE_BEGIN(MSG_BROADCAST, gmsgDrawFX);
		WRITE_BYTE(FX_EXPLOSION_WALL);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_COORD(m_iTrace.vecPlaneNormal.x*1000.0);//-1~1
		WRITE_COORD(m_iTrace.vecPlaneNormal.y*1000.0);//-1~1
		WRITE_COORD(m_iTrace.vecPlaneNormal.z*1000.0);//-1~1
		MESSAGE_END();
	}
	else
	{
		MESSAGE_BEGIN(MSG_BROADCAST, gmsgDrawFX);
		WRITE_BYTE(FX_EXPLOSION_MIDAIR);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		MESSAGE_END();
	}*/

	pev->nextthink = gpGlobals->time + RANDOM_FLOAT(0.025, 0.05);
	SetThink(&CGrenade::CKFSmoke);
}

void CGrenade::CKFSmoke(void)
{
	switch(RANDOM_LONG(0,2))
	{
	case 0:EMIT_SOUND(ENT(pev), CHAN_STATIC, "CKF_III/explode1.wav", 1.0, 0.80);break;
	case 1:EMIT_SOUND(ENT(pev), CHAN_STATIC, "CKF_III/explode2.wav", 1.0, 0.80);break;
	case 2:EMIT_SOUND(ENT(pev), CHAN_STATIC, "CKF_III/explode3.wav", 1.0, 0.80);break;
	}

	if (!RANDOM_LONG(0, 1))
		UTIL_DecalTrace(&m_iTrace, DECAL_SCORCH1);
	else
		UTIL_DecalTrace(&m_iTrace, DECAL_SCORCH2);

	if(m_iTrace.flFraction < 1)
	{
		MESSAGE_BEGIN(MSG_BROADCAST, gmsgDrawFX);
		WRITE_BYTE(FX_EXPLOSION_WALL);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_COORD(m_iTrace.vecPlaneNormal.x*1000.0);//-1~1
		WRITE_COORD(m_iTrace.vecPlaneNormal.y*1000.0);//-1~1
		WRITE_COORD(m_iTrace.vecPlaneNormal.z*1000.0);//-1~1
		MESSAGE_END();
	}
	else
	{
		MESSAGE_BEGIN(MSG_BROADCAST, gmsgDrawFX);
		WRITE_BYTE(FX_EXPLOSION_MIDAIR);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		MESSAGE_END();
	}

	/*if (UTIL_PointContents(pev->origin) != CONTENTS_WATER)
	{
		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, pev->origin);
		WRITE_BYTE(TE_SMOKE);
		WRITE_COORD(pev->origin.x);
		WRITE_COORD(pev->origin.y);
		WRITE_COORD(pev->origin.z);
		WRITE_SHORT(g_sModelIndexSmoke);
		WRITE_BYTE(25 + RANDOM_FLOAT(0, 10));
		WRITE_BYTE(10);
		MESSAGE_END();
	}
	else
		UTIL_Bubbles(pev->origin - Vector(64, 64, 64), pev->origin + Vector(64, 64, 64), 100);
	*/

	switch (RANDOM_LONG(0, 2))
	{
		case 0: EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris1.wav", 0.55, ATTN_NORM); break;
		case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris2.wav", 0.55, ATTN_NORM); break;
		case 2: EMIT_SOUND(ENT(pev), CHAN_VOICE, "weapons/debris3.wav", 0.55, ATTN_NORM); break;
	}
	SetThink(&CBaseEntity::SUB_Remove);
	pev->nextthink = gpGlobals->time + 0.1;
}

/*TYPEDESCRIPTION CGrenade::m_SaveData[] =
{
	DEFINE_FIELD(CGrenade, m_fAttenu, FIELD_FLOAT),
	DEFINE_FIELD(CGrenade, m_flNextFreq, FIELD_TIME),
	DEFINE_FIELD(CGrenade, m_flC4Blow, FIELD_TIME),
	DEFINE_FIELD(CGrenade, m_flNextFreqInterval, FIELD_TIME),
	DEFINE_FIELD(CGrenade, m_flNextBeep, FIELD_TIME),
	DEFINE_FIELD(CGrenade, m_flDefuseCountDown, FIELD_TIME),
	DEFINE_FIELD(CGrenade, m_flNextBlink, FIELD_TIME),
	DEFINE_FIELD(CGrenade, m_pentCurBombTarget, FIELD_EDICT),
	DEFINE_FIELD(CGrenade, m_sBeepName, FIELD_POINTER),
	DEFINE_FIELD(CGrenade, m_bIsC4, FIELD_BOOLEAN),
	DEFINE_FIELD(CGrenade, m_bStartDefuser, FIELD_BOOLEAN),
	DEFINE_FIELD(CGrenade, m_SGSmoke, FIELD_INTEGER),
	DEFINE_FIELD(CGrenade, m_bJustBlew, FIELD_BOOLEAN),
	DEFINE_FIELD(CGrenade, m_bLightSmoke, FIELD_BOOLEAN),
	DEFINE_FIELD(CGrenade, m_usEvent, FIELD_INTEGER),
};

IMPLEMENT_SAVERESTORE(CGrenade, CBaseMonster);*/