#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "monsters.h"
#include "soundent.h"
#include "decals.h"
#include "animation.h"
#include "weapons.h"
#include "func_break.h"
#include "pm_materials.h"
#include "player.h"
#include "buildable.h"

extern DLL_GLOBAL Vector g_vecAttackDir;
extern entvars_t *g_pevLastInflictor;

#define GERMAN_GIB_COUNT 4
#define HUMAN_GIB_COUNT 6
#define ALIEN_GIB_COUNT 4

void CGib::LimitVelocity(void)
{
	float length = pev->velocity.Length();

	if (length > 1500)
		pev->velocity = pev->velocity.Normalize() * 1500;
}

void CGib::SpawnStickyGibs(entvars_t *pevVictim, Vector vecOrigin, int cGibs)
{
	if (g_Language == LANGUAGE_GERMAN)
		return;

	for (int i = 0; i < cGibs; i++)
	{
		CGib *pGib = GetClassPtr((CGib *)NULL);
		pGib->Spawn("models/stickygib.mdl");
		pGib->pev->body = RANDOM_LONG(0, 2);

		if (pevVictim)
		{
			pGib->pev->origin.x = vecOrigin.x + RANDOM_FLOAT(-3, 3);
			pGib->pev->origin.y = vecOrigin.y + RANDOM_FLOAT(-3, 3);
			pGib->pev->origin.z = vecOrigin.z + RANDOM_FLOAT(-3, 3);
			pGib->pev->velocity = g_vecAttackDir * -1;
			pGib->pev->velocity.x += RANDOM_FLOAT(-0.15, 0.15);
			pGib->pev->velocity.y += RANDOM_FLOAT(-0.15, 0.15);
			pGib->pev->velocity.z += RANDOM_FLOAT(-0.15, 0.15);
			pGib->pev->velocity = pGib->pev->velocity * 900;
			pGib->pev->avelocity.x = RANDOM_FLOAT(250, 400);
			pGib->pev->avelocity.y = RANDOM_FLOAT(250, 400);
			pGib->m_bloodColor = (CBaseEntity::Instance(pevVictim))->BloodColor();

			if (pevVictim->health > -50)
				pGib->pev->velocity = pGib->pev->velocity * 0.7;
			else if (pevVictim->health > -200)
				pGib->pev->velocity = pGib->pev->velocity * 2;
			else
				pGib->pev->velocity = pGib->pev->velocity * 4;

			pGib->pev->movetype = MOVETYPE_TOSS;
			pGib->pev->solid = SOLID_BBOX;
			UTIL_SetSize(pGib->pev, Vector(0, 0 ,0), Vector(0, 0, 0));
			pGib->SetTouch(&CGib::StickyGibTouch);
			pGib->SetThink(NULL);
		}

		pGib->LimitVelocity();
	}
}

void CGib::SpawnHeadGib(entvars_t *pevVictim)
{
	CGib *pGib = GetClassPtr((CGib *)NULL);

	if (g_Language == LANGUAGE_GERMAN)
	{
		pGib->Spawn("models/germangibs.mdl");
		pGib->pev->body = 0;
	}
	else
	{
		pGib->Spawn("models/hgibs.mdl");
		pGib->pev->body = 0;
	}

	if (pevVictim)
	{
		pGib->pev->origin = pevVictim->origin + pevVictim->view_ofs;
		edict_t *pentPlayer = FIND_CLIENT_IN_PVS(pGib->edict());

		if (RANDOM_LONG(0, 100) <= 5 && pentPlayer)
		{
			entvars_t *pevPlayer = VARS(pentPlayer);
			pGib->pev->velocity = ((pevPlayer->origin + pevPlayer->view_ofs) - pGib->pev->origin).Normalize() * 300;
			pGib->pev->velocity.z += 100;
		}
		else
			pGib->pev->velocity = Vector(RANDOM_FLOAT(-100, 100), RANDOM_FLOAT(-100, 100), RANDOM_FLOAT(200, 300));

		pGib->pev->avelocity.x = RANDOM_FLOAT(100, 200);
		pGib->pev->avelocity.y = RANDOM_FLOAT(100, 300);
		pGib->m_bloodColor = (CBaseEntity::Instance(pevVictim))->BloodColor();

		if (pevVictim->health > -50)
			pGib->pev->velocity = pGib->pev->velocity * 0.7;
		else if (pevVictim->health > -200)
			pGib->pev->velocity = pGib->pev->velocity * 2;
		else
			pGib->pev->velocity = pGib->pev->velocity * 4;
	}
	pGib->LimitVelocity();
}

void CGib::SpawnRandomGibs(entvars_t *pevVictim, int cGibs, int human)
{
	for (int cSplat = 0; cSplat < cGibs; cSplat++)
	{
		CGib *pGib = GetClassPtr((CGib *)NULL);

		if (g_Language == LANGUAGE_GERMAN)
		{
			pGib->Spawn("models/germangibs.mdl");
			pGib->pev->body = RANDOM_LONG(0, GERMAN_GIB_COUNT - 1);
		}
		else
		{
			if (human)
			{
				pGib->Spawn("models/hgibs.mdl");
				pGib->pev->body = RANDOM_LONG(1, HUMAN_GIB_COUNT - 1);
			}
			else
			{
				pGib->Spawn("models/agibs.mdl");
				pGib->pev->body = RANDOM_LONG(0, ALIEN_GIB_COUNT - 1);
			}
		}

		if (pevVictim)
		{
			pGib->pev->origin.x = pevVictim->absmin.x + pevVictim->size.x * RANDOM_FLOAT(0, 1);
			pGib->pev->origin.y = pevVictim->absmin.y + pevVictim->size.y * RANDOM_FLOAT(0, 1);
			pGib->pev->origin.z = pevVictim->absmin.z + pevVictim->size.z * RANDOM_FLOAT(0, 1) + 1;
			pGib->pev->velocity = g_vecAttackDir * -1;
			pGib->pev->velocity.x += RANDOM_FLOAT(-0.25, 0.25);
			pGib->pev->velocity.y += RANDOM_FLOAT(-0.25, 0.25);
			pGib->pev->velocity.z += RANDOM_FLOAT(-0.25, 0.25);
			pGib->pev->velocity = pGib->pev->velocity * RANDOM_FLOAT(300, 400);
			pGib->pev->avelocity.x = RANDOM_FLOAT(100, 200);
			pGib->pev->avelocity.y = RANDOM_FLOAT(100, 300);
			pGib->m_bloodColor = (CBaseEntity::Instance(pevVictim))->BloodColor();

			if (pevVictim->health > -50)
				pGib->pev->velocity = pGib->pev->velocity * 0.7;
			else if (pevVictim->health > -200)
				pGib->pev->velocity = pGib->pev->velocity * 2;
			else
				pGib->pev->velocity = pGib->pev->velocity * 4;

			pGib->pev->solid = SOLID_BBOX;
			UTIL_SetSize(pGib->pev, Vector(0, 0, 0), Vector(0, 0, 0));
		}

		pGib->LimitVelocity();
	}
}

BOOL CBaseMonster::HasHumanGibs(void)
{
	int myClass = Classify();

	if (myClass == CLASS_HUMAN_MILITARY || myClass == CLASS_PLAYER_ALLY || myClass == CLASS_HUMAN_PASSIVE || myClass == CLASS_PLAYER)
		return TRUE;

	return FALSE;
}

BOOL CBaseMonster::HasAlienGibs(void)
{
	int myClass = Classify();

	if (myClass == CLASS_ALIEN_MILITARY || myClass == CLASS_ALIEN_MONSTER || myClass == CLASS_ALIEN_PASSIVE || myClass == CLASS_INSECT || myClass == CLASS_ALIEN_PREDATOR || myClass == CLASS_ALIEN_PREY)
		return TRUE;

	return FALSE;
}

void CBaseMonster::FadeMonster(void)
{
	StopAnimation();
	pev->velocity = g_vecZero;
	pev->movetype = MOVETYPE_NONE;
	pev->avelocity = g_vecZero;
	pev->animtime = gpGlobals->time;
	pev->effects |= EF_NOINTERP;
	SUB_StartFadeOut();
}

void CBaseMonster::GibMonster(void)
{
	BOOL gibbed = FALSE;
	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "common/bodysplat.wav", 1, ATTN_NORM);

	if (HasHumanGibs())
	{
		if (CVAR_GET_FLOAT("violence_hgibs") != 0)
		{
			CGib::SpawnHeadGib(pev);
			CGib::SpawnRandomGibs(pev, 4, 1);
		}

		gibbed = TRUE;
	}
	else if (HasAlienGibs())
	{
		if (CVAR_GET_FLOAT("violence_agibs") != 0)
			CGib::SpawnRandomGibs(pev, 4, 0);

		gibbed = TRUE;
	}

	if (!IsPlayer())
	{
		if (gibbed)
		{
			SetThink(&CBaseEntity::SUB_Remove);
			pev->nextthink = gpGlobals->time;
		}
		else
			FadeMonster();
	}
}

Activity CBaseMonster::GetDeathActivity(void)
{
	if (pev->deadflag != DEAD_NO)
		return m_IdealActivity;

	Vector vecSrc = Center();
	BOOL fTriedDirection = FALSE;
	Activity deathActivity = ACT_DIESIMPLE;
	TraceResult tr;

	UTIL_MakeVectors(pev->angles);
	float flDot = DotProduct(gpGlobals->v_forward, g_vecAttackDir * -1);

	switch (m_LastHitGroup)
	{
		case HITGROUP_HEAD: deathActivity = ACT_DIE_HEADSHOT; break;
		case HITGROUP_STOMACH: deathActivity = ACT_DIE_GUTSHOT; break;

		case HITGROUP_GENERIC:
		default:
		{
			fTriedDirection = TRUE;

			if (flDot > 0.3)
				deathActivity = ACT_DIEFORWARD;
			else if (flDot <= -0.3)
				deathActivity = ACT_DIEBACKWARD;

			break;
		}
	}

	if (LookupActivity(deathActivity) == ACTIVITY_NOT_AVAILABLE)
	{
		if (!fTriedDirection)
		{
			if (flDot > 0.3)
				deathActivity = ACT_DIEFORWARD;
			else if (flDot <= -0.3)
				deathActivity = ACT_DIEBACKWARD;
		}
		else
			deathActivity = ACT_DIESIMPLE;
	}

	if (LookupActivity(deathActivity) == ACTIVITY_NOT_AVAILABLE)
		deathActivity = ACT_DIESIMPLE;

	if (deathActivity == ACT_DIEFORWARD)
	{
		UTIL_TraceHull(vecSrc, vecSrc + gpGlobals->v_forward * 64, dont_ignore_monsters, head_hull, edict(), &tr);

		if (tr.flFraction != 1)
			deathActivity = ACT_DIESIMPLE;
	}

	if (deathActivity == ACT_DIEBACKWARD)
	{
		UTIL_TraceHull(vecSrc, vecSrc - gpGlobals->v_forward * 64, dont_ignore_monsters, head_hull, edict(), &tr);

		if (tr.flFraction != 1)
			deathActivity = ACT_DIESIMPLE;
	}

	return deathActivity;
}

Activity CBaseMonster::GetSmallFlinchActivity(void)
{
	Activity flinchActivity;
	BOOL fTriedDirection = FALSE;
	UTIL_MakeVectors(pev->angles);
	float flDot = DotProduct(gpGlobals->v_forward, g_vecAttackDir * -1);

	switch (m_LastHitGroup)
	{
		case HITGROUP_HEAD: flinchActivity = ACT_FLINCH_HEAD; break;
		case HITGROUP_STOMACH: flinchActivity = ACT_FLINCH_STOMACH; break;
		case HITGROUP_LEFTARM: flinchActivity = ACT_FLINCH_LEFTARM; break;
		case HITGROUP_RIGHTARM: flinchActivity = ACT_FLINCH_RIGHTARM; break;
		case HITGROUP_LEFTLEG: flinchActivity = ACT_FLINCH_LEFTLEG; break;
		case HITGROUP_RIGHTLEG: flinchActivity = ACT_FLINCH_RIGHTLEG; break;

		case HITGROUP_GENERIC:
		default: flinchActivity = ACT_SMALL_FLINCH; break;
	}

	if (LookupActivity(flinchActivity) == ACTIVITY_NOT_AVAILABLE)
		flinchActivity = ACT_SMALL_FLINCH;

	return flinchActivity;
}

void CBaseMonster::BecomeDead(void)
{
	pev->takedamage = DAMAGE_YES;
	pev->health = pev->max_health / 2;
	pev->max_health = 5;
	pev->movetype = MOVETYPE_TOSS;
}

BOOL CBaseMonster::ShouldGibMonster(int iGib)
{
	if ((iGib == GIB_NORMAL && pev->health < GIB_HEALTH_VALUE) || iGib == GIB_ALWAYS)
		return TRUE;

	return FALSE;
}

void CBaseMonster::CallGibMonster(void)
{
	BOOL fade = FALSE;

	if (HasHumanGibs())
	{
		if (!CVAR_GET_FLOAT("violence_hgibs"))
			fade = TRUE;
	}
	else if (HasAlienGibs())
	{
		if (!CVAR_GET_FLOAT("violence_agibs"))
			fade = TRUE;
	}

	pev->takedamage = DAMAGE_NO;
	pev->solid = SOLID_NOT;

	if (!fade)
	{
		pev->effects = EF_NODRAW;
		GibMonster();
	}
	else
		FadeMonster();

	pev->deadflag = DEAD_DEAD;
	FCheckAITrigger();

	if (pev->health < -99)
		pev->health = 0;

	if (ShouldFadeOnDeath() && !fade)
		UTIL_Remove(this);
}

void CBaseMonster::Killed(entvars_t *pevAttacker, int iGib)
{
	if (HasMemory(bits_MEMORY_KILLED))
	{
		if (ShouldGibMonster(iGib))
			CallGibMonster();

		return;
	}

	Remember(bits_MEMORY_KILLED);
	EMIT_SOUND(ENT(pev), CHAN_WEAPON, "common/null.wav", VOL_NORM, ATTN_NORM);
	m_IdealMonsterState = MONSTERSTATE_DEAD;
	SetConditions(bits_COND_LIGHT_DAMAGE);

	CBaseEntity *pOwner = CBaseEntity::Instance(pev->owner);

	if (pOwner)
		pOwner->DeathNotice(pev);

	if (ShouldGibMonster(iGib))
	{
		CallGibMonster();
		return;
	}
	else if (pev->flags & FL_MONSTER)
	{
		SetTouch(NULL);
		BecomeDead();
	}

	if (pev->health < -99)
		pev->health = 0;

	m_IdealMonsterState = MONSTERSTATE_DEAD;
}

void CBaseEntity::SUB_StartFadeOut(void)
{
	if (pev->rendermode == kRenderNormal)
	{
		pev->renderamt = 255;
		pev->rendermode = kRenderTransTexture;
	}

	pev->solid = SOLID_NOT;
	pev->avelocity = g_vecZero;
	pev->nextthink = gpGlobals->time + 0.1;
	SetThink(&CBaseEntity::SUB_FadeOut);
}

void CBaseEntity::SUB_FadeOut(void)
{
	if (pev->renderamt > 7)
	{
		pev->renderamt -= 7;
		pev->nextthink = gpGlobals->time + 0.1;
	}
	else
	{
		pev->renderamt = 0;
		pev->nextthink = gpGlobals->time + 0.2;
		SetThink(&CBaseEntity::SUB_Remove);
	}
}

void CGib::WaitTillLand(void)
{
	if (!IsInWorld())
	{
		UTIL_Remove(this);
		return;
	}

	if (pev->velocity == g_vecZero)
	{
		SetThink(&CBaseEntity::SUB_StartFadeOut);
		pev->nextthink = gpGlobals->time + m_lifeTime;

		if (m_bloodColor != DONT_BLEED)
			CSoundEnt::InsertSound(bits_SOUND_MEAT, pev->origin, 384, 25);
	}
	else
		pev->nextthink = gpGlobals->time + 0.5;
}

void CGib::BounceGibTouch(CBaseEntity *pOther)
{
	if (pev->flags & FL_ONGROUND)
	{
		pev->velocity = pev->velocity * 0.9;
		pev->angles.x = 0;
		pev->angles.z = 0;
		pev->avelocity.x = 0;
		pev->avelocity.z = 0;
	}
	else
	{
		if (g_Language != LANGUAGE_GERMAN && m_cBloodDecals > 0 && m_bloodColor != DONT_BLEED)
		{
			TraceResult tr;
			Vector vecSpot = pev->origin + Vector(0, 0, 8);
			UTIL_TraceLine(vecSpot, vecSpot + Vector(0, 0, -24), ignore_monsters, ENT(pev), &tr);
			UTIL_BloodDecalTrace(&tr, m_bloodColor);
			m_cBloodDecals--;
		}

		if (m_material != matNone && !RANDOM_LONG(0, 2))
		{
			float zvel = fabs(pev->velocity.z);
			float volume = 0.8 * min(1, zvel / 450);
			CBreakable::MaterialSoundRandom(edict(), (Materials)m_material, volume);
		}
	}
}

void CGib::StickyGibTouch(CBaseEntity *pOther)
{
	SetThink(&CBaseEntity::SUB_Remove);
	pev->nextthink = gpGlobals->time + 10;

	if (!FClassnameIs(pOther->pev, "worldspawn"))
	{
		pev->nextthink = gpGlobals->time;
		return;
	}

	TraceResult tr;
	UTIL_TraceLine(pev->origin, pev->origin + pev->velocity * 32, ignore_monsters, ENT(pev), &tr);
	UTIL_BloodDecalTrace(&tr, m_bloodColor);

	pev->velocity = tr.vecPlaneNormal * -1;
	pev->angles = UTIL_VecToAngles(pev->velocity);
	pev->velocity = g_vecZero;
	pev->avelocity = g_vecZero;
	pev->movetype = MOVETYPE_NONE;
}

void CGib::Spawn(const char *szGibModel)
{
	pev->movetype = MOVETYPE_BOUNCE;
	pev->friction = 0.55;
	pev->renderamt = 255;
	pev->rendermode = kRenderNormal;
	pev->renderfx = kRenderFxNone;
	pev->solid = SOLID_SLIDEBOX;
	pev->classname = MAKE_STRING("gib");

	SET_MODEL(ENT(pev), szGibModel);
	UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));

	pev->nextthink = gpGlobals->time + 4;
	m_lifeTime = 25;
	SetThink(&CGib::WaitTillLand);
	SetTouch(&CGib::BounceGibTouch);
	m_material = matNone;
	m_cBloodDecals = 5;
}

int CBaseMonster::TakeHealth(float flHealth, int bitsDamageType)
{
	if (!pev->takedamage)
		return 0;

	m_bitsDamageType &= ~(bitsDamageType & ~DMG_TIMEBASED);
	return CBaseEntity::TakeHealth(flHealth, bitsDamageType);
}

int CBaseMonster::TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType, int iCrit)
{
	if (!pev->takedamage)
		return 0;

	if (!IsAlive())
		return DeadTakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);

	if (pev->deadflag == DEAD_NO)
		PainSound();

	float flTake = flDamage;
	m_bitsDamageType |= bitsDamageType;
	Vector vecDir = Vector(0, 0, 0);

	if (!FNullEnt(pevInflictor))
	{
		CBaseEntity *pInflictor = CBaseEntity::Instance(pevInflictor);

		if (pInflictor)
		{
			vecDir = (pInflictor->Center() - Vector(0, 0, 10) - Center()).Normalize();
			vecDir = g_vecAttackDir = vecDir.Normalize();
		}
	}

	if (IsPlayer())
	{
		if (pevInflictor)
			pev->dmg_inflictor = ENT(pevInflictor);

		pev->dmg_take += flTake;
	}

	pev->health -= flTake;

	if (m_MonsterState == MONSTERSTATE_SCRIPT)
	{
		SetConditions(bits_COND_LIGHT_DAMAGE);
		return 0;
	}

	if (pev->health <= 0)
	{
		g_pevLastInflictor = pevInflictor;

		if (bitsDamageType & DMG_ALWAYSGIB)
			Killed(pevAttacker, GIB_ALWAYS);
		else if (bitsDamageType & DMG_NEVERGIB)
			Killed(pevAttacker, GIB_NEVER);
		else
			Killed(pevAttacker, GIB_NORMAL);

		g_pevLastInflictor = NULL;
		return 0;
	}

	if ((pev->flags & FL_MONSTER) && !FNullEnt(pevAttacker))
	{
		if (pevAttacker->flags & (FL_MONSTER | FL_CLIENT))
		{
			if (pevInflictor)
			{
				if (m_hEnemy == NULL || pevInflictor == m_hEnemy->pev || !HasConditions(bits_COND_SEE_ENEMY))
					m_vecEnemyLKP = pevInflictor->origin;
			}
			else
				m_vecEnemyLKP = pev->origin + (g_vecAttackDir * 64);

			MakeIdealYaw(m_vecEnemyLKP);

			if (flDamage > 0)
				SetConditions(bits_COND_LIGHT_DAMAGE);

			if (flDamage >= 20)
				SetConditions(bits_COND_HEAVY_DAMAGE);
		}
	}

	return 1;
}

int CBaseMonster::DeadTakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType)
{
	Vector vecDir = Vector(0, 0, 0);

	if (!FNullEnt(pevInflictor))
	{
		CBaseEntity *pInflictor = CBaseEntity::Instance(pevInflictor);

		if (pInflictor)
		{
			vecDir = (pInflictor->Center() - Vector(0, 0, 10) - Center()).Normalize();
			vecDir = g_vecAttackDir = vecDir.Normalize();
		}
	}

	if (bitsDamageType & DMG_GIB_CORPSE)
	{
		if (pev->health <= flDamage)
		{
			pev->health = -50;
			Killed(pevAttacker, GIB_ALWAYS);
			return 0;
		}

		pev->health -= flDamage * 0.1;
	}

	return 1;
}

float CBaseMonster::DamageForce(float damage)
{
	float force = damage * ((32 * 32 * 72.0) / (pev->size.x * pev->size.y * pev->size.z)) * 5;

	if (force > 1000)
		force = 1000;

	return force;
}

void RadiusFlash(Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage)
{
	CBaseEntity *pEntity = NULL;
	float flRadius = 1500;
	float falloff = flDamage / flRadius;
	int bInWater = (UTIL_PointContents(vecSrc) == CONTENTS_WATER);

	vecSrc.z += 1;

	while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flRadius)) != NULL)
	{
		if (!pEntity->IsPlayer())
			break;

		if (pEntity->pev->takedamage == DAMAGE_NO || pEntity->pev->deadflag != DEAD_NO)
			continue;

		if (bInWater && !pEntity->pev->waterlevel)
			continue;

		if (!bInWater && pEntity->pev->waterlevel == 3)
			continue;

		TraceResult tr;
		Vector vecSpot = pEntity->BodyTarget(vecSrc);
		UTIL_TraceLine(vecSrc, vecSpot, dont_ignore_monsters, ENT(pevInflictor), &tr);

		if (tr.flFraction == 1 || tr.pHit == pEntity->edict())
		{
			if (tr.fStartSolid)
			{
				tr.vecEndPos = vecSrc;
				tr.flFraction = 0;
			}

			float flAdjustedDamage = (vecSrc - tr.vecEndPos).Length() * falloff;
			flAdjustedDamage = flDamage - flAdjustedDamage;

			if (flAdjustedDamage < 0)
				flAdjustedDamage = 0;

			UTIL_MakeVectors(pEntity->pev->v_angle);
			Vector vecLOS = vecSrc - pEntity->EarPosition();
			float flDot = DotProduct(vecLOS, gpGlobals->v_forward);

			if (flDot >= 0)
				UTIL_ScreenFade(pEntity, Vector(255, 255, 255), flAdjustedDamage * 3, flAdjustedDamage / 1.5, 255, 0);
			else
				UTIL_ScreenFade(pEntity, Vector(255, 255, 255), flAdjustedDamage * 1.75, flAdjustedDamage / 3.5, 200, 0);
		}
	}
}

void CKFRadiusDamage(Vector vecSrc, entvars_t *pevDirectHit, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType, float flForce, float flForceRadius, int iCrit)
{
	if(flForceRadius < flRadius) flForceRadius = flRadius;

	CBaseEntity *pEntity = NULL;

	vecSrc.z += 1;

	if (!pevAttacker) pevAttacker = pevInflictor;

	int iTeam = 0;
	int iFinalCrit;

	BOOL bIsPlayer = FALSE;
	BOOL bTakeForce = FALSE;

	CBaseEntity *pAttacker = CBaseEntity::Instance( pevAttacker );

	CBasePlayer *pPlayer = NULL;

	if(pAttacker->IsPlayer())
	{
		iTeam = ((CBasePlayer *)pAttacker)->m_iTeam;
	}

	while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flForceRadius*1.2)) != NULL)
	{
		if (pEntity->pev->takedamage == DAMAGE_NO)
			continue;

		if (iClassIgnore != CLASS_NONE && pEntity->Classify() == iClassIgnore)
			continue;

		bIsPlayer = (pEntity->IsPlayer() && pEntity->IsAlive()) ? TRUE : FALSE;

		if(bIsPlayer)
		{
			pPlayer = (CBasePlayer *)pEntity;
			if(pPlayer->m_iTeam == iTeam && pEntity->pev != pevAttacker)
				continue;
			if(bitsDamageType & DMG_NOSELFDMG && (pEntity->pev) == pevAttacker)
				continue;
		}
		else if(pEntity->Classify() == CLASS_PROJECTILE)
		{
			CGrenade *pProj = (CGrenade *)pEntity;
			if ( pProj->m_iTeam == iTeam )
				continue;
		}
		else if(pEntity->Classify() == CLASS_BUILDABLE)
		{
			CBaseBuildable *pBuild = (CBaseBuildable *)pEntity;
			if ( pBuild->m_iTeam == iTeam )
				continue;
		}

		iFinalCrit = iCrit + pEntity->GetCriticalHit();

		TraceResult tr;
		Vector vecSpot = pEntity->BodyTarget(vecSrc);

		UTIL_TraceLine(vecSrc, vecSpot, dont_ignore_monsters, ENT(pevInflictor), &tr);

		float flDistance = ( vecSpot - vecSrc ).Length();

		if(pEntity->IsBSPModel()) flDistance /= 4;

		if(pEntity->IsPlayer() && (bitsDamageType & DMG_SHAKE) && (pEntity->pev->flags & FL_ONGROUND))
		{//shake it off
			UTIL_SingleScreenShake((5.0+5.0*flDamage/100) * (1 - min(flDistance / flForceRadius, 0.8) ), 1.6, 0.8, pEntity);
		}

		if(flDistance > flForceRadius)
			continue;

		if (tr.flFraction == 1 || tr.pHit == pEntity->edict())
		{
			if (tr.fStartSolid)
			{
				tr.vecEndPos = vecSrc;
				tr.flFraction = 0;
			}

			int iRocketJump = 0;

			if(flForce > 0)
			{
				if(pEntity->pev == pevAttacker) flDistance = min(flDistance, flForceRadius/4);

				float flAdjustedForce = (1 - flDistance / flForceRadius ) * flForce;

				if (flAdjustedForce < 0)
					flAdjustedForce = 0;

				if(bIsPlayer)
				{
					flAdjustedForce *= pPlayer->GetKnockBack();
					if(pPlayer->m_iDisguise == DISGUISE_YES)
						flAdjustedForce = 0;
					if(pEntity->pev->bInDuck || pEntity->pev->button & IN_DUCK)
						flAdjustedForce *= 1.15;
					if(pEntity->pev->button & IN_JUMP)
						flAdjustedForce *= 1.15;

					Vector vecDir = Vector( 0, 0, 0 );

					vecDir = ( vecSpot - vecSrc ).Normalize();

					if(pevDirectHit == pEntity->pev && flAdjustedForce > 0)
					{
						pEntity->pev->velocity.z += 325 * max(min(fabs(vecSrc.z+-vecSpot.z-24) / 32, 1.0f), -1.0f);
					}
					else if(pEntity->pev != pevAttacker)
					{
						flAdjustedForce *= RANDOM_FLOAT(2.3, 2.5);
					}
					if( pPlayer->m_iRocketJump ) flAdjustedForce *= 1.5;

					pEntity->pev->velocity = pEntity->pev->velocity + vecDir * flAdjustedForce;

					if(pEntity->pev->velocity.Length2D() > 400 || pEntity->pev->velocity.z > 400)
					{
						if(pEntity->pev == pevAttacker) iRocketJump = 1;
					}

					if(iRocketJump) pPlayer->m_iRocketJump = 1;
				}
				else if(pEntity->Classify() == CLASS_PROJECTILE)
				{
					CGrenade *pProj = (CGrenade *)pEntity;
					if(!(pProj->m_iPjFlags & PJ_AIRBLAST_DEFLECTABLE) || pEntity->pev->velocity.Length() > 400)
						continue;
					if(pProj->m_iPjFlags & PJ_AIRBLAST_ROTATE)
						pEntity->pev->avelocity.x = RANDOM_FLOAT(-720, 720);
					if(pProj->m_iPjFlags & PJ_AIRBLAST_UPWARD)
					{
						pEntity->pev->movetype = MOVETYPE_TOSS;
						pEntity->pev->velocity.z = 100;
						Vector vecDir = Vector( 0, 0, 0 );
						vecDir = ( vecSpot - vecSrc + Vector ( 0, 0, 2 ) ).Normalize();
						pEntity->pev->velocity = pEntity->pev->velocity + vecDir * flAdjustedForce;
					}
					continue;
				}
				else if(pEntity->Classify() == CLASS_WEAPONBOX)
				{
					pEntity->pev->avelocity.y = RANDOM_FLOAT(-720, 720);
					pEntity->pev->velocity.z = 100;
					Vector vecDir = Vector( 0, 0, 0 );
					vecDir = ( vecSpot - vecSrc + Vector ( 0, 0, 2 ) ).Normalize();
					pEntity->pev->velocity = pEntity->pev->velocity + vecDir * flAdjustedForce * 1.5;
					continue;
				}
			}

			if(flDistance > flRadius) continue;

			if(pEntity->Classify() == CLASS_PROJECTILE)
				continue;//Radius damage can not do damage to projectile

			float flDamageModifier = 1.0;

			float flAdjustedDamage = 0;

			if(pEntity->pev == pevAttacker)
			{
				flAdjustedDamage = flDamage*0.9 - flDamage * 0.6 * flDistance / flRadius;
				flDamageModifier = RANDOM_FLOAT(0.85, 1.0);
				// https://wiki.teamfortress.com/wiki/Jumping#Soldier_jumps
				// Soldier has a 40% damage resistance against any rocket he fires that hurts him while he is in the air.
				if(((CBasePlayer *)pEntity)->m_iRocketJump)
					flDamageModifier *= 1.0 - 0.4;
			}
			else
			{
				flAdjustedDamage = flDamage - flDamage * 0.5 * flDistance / flRadius;
				if(bitsDamageType & DMG_RANGE)
				{
					float flPlayerDistance = (pevAttacker->origin - vecSpot).Length();

					if(flPlayerDistance < DIST_MEDIUM) flDamageModifier = -0.2*flPlayerDistance/DIST_MEDIUM+1.2;
					else if(flPlayerDistance < DIST_LONG) flDamageModifier = sin(M_PI/2*flPlayerDistance/DIST_LONG + M_PI/2) + 0.5;
					else flDamageModifier = 0.5;
					if(iCrit == 1 && flDistance >= DIST_MEDIUM) flDamageModifier = 1.0;
				}
				if(bIsPlayer)
				{
					if(iCrit == 2) flDamageModifier = 3;
					else if(iCrit == 1) flDamageModifier *= 1.35f;
				}
			}

			flAdjustedDamage *= flDamageModifier;

			if (flAdjustedDamage < 0)
				flAdjustedDamage = 0;

			ClearMultiDamage();
			pEntity->TraceAttack(pevInflictor, flAdjustedDamage, (tr.vecEndPos - vecSrc).Normalize(), &tr, bitsDamageType, iCrit);
			ApplyMultiDamage(pevInflictor, pevAttacker);
		}
	}
}

void CKFAirblast(Vector vecSrc, Vector vecDirShooting, entvars_t *pevInflictor, entvars_t *pevAttacker, int iClassIgnore, float flForce, float flForceRadius)
{
	CBaseEntity *pEntity = NULL;

	if (!pevAttacker) pevAttacker = pevInflictor;

	int iTeam = 0;

	bool bIsPlayer = false;
	bool bDeflected = false;

	CBaseEntity *pAttacker = CBaseEntity::Instance( pevAttacker );

	CBasePlayer *pPlayer = NULL;

	if(pAttacker->IsPlayer())
	{
		iTeam = ((CBasePlayer *)pAttacker)->m_iTeam;
	}

	Vector vecHorizonal = vecDirShooting;
	vecHorizonal.z = max(vecHorizonal.z, 0);
	vecHorizonal = vecHorizonal.Normalize();

	while ((pEntity = UTIL_FindEntityInSphere(pEntity, vecSrc, flForceRadius)) != NULL)
	{
		if (pEntity->pev->takedamage == DAMAGE_NO)
			continue;

		if (iClassIgnore != CLASS_NONE && pEntity->Classify() == iClassIgnore)
			continue;

		if(pEntity->IsBSPModel())
			continue;

		bIsPlayer = (pEntity->IsPlayer() && pEntity->IsAlive()) ? true : false;

		if(bIsPlayer)
		{
			pPlayer = (CBasePlayer *)pEntity;
			if(pEntity->pev == pevAttacker) continue;
		}
		else if(pEntity->Classify() == CLASS_PROJECTILE)
		{
			CGrenade *pProj = (CGrenade *)pEntity;
			if(pProj->m_iTeam == iTeam) continue;
		}
		else
		{
			continue;
		}

		TraceResult tr;
		Vector vecSpot = pEntity->BodyTarget(vecSrc);

		UTIL_TraceLine(vecSrc, vecSpot, ignore_monsters, ENT(pevInflictor), &tr);

		if (tr.flFraction == 1 || tr.pHit == pEntity->edict())
		{
			if (tr.fStartSolid)
			{
				tr.vecEndPos = vecSrc;
				tr.flFraction = 0;
			}

			if(bIsPlayer)
			{
				//put out friendly player's fire
				if(pPlayer->m_iTeam == iTeam)
				{
					pPlayer->m_Cond.AfterBurn.Remove();
				}
				else
				{
					pEntity->pev->punchangle.x = RANDOM_FLOAT(-20, -10);
					pEntity->pev->velocity.z = 300;
					pEntity->pev->velocity = pEntity->pev->velocity + vecHorizonal * flForce;
				}
			}
			else if(pEntity->Classify() == CLASS_PROJECTILE)
			{
				CGrenade *pProj = (CGrenade *)pEntity;
				if(!( pProj->m_iPjFlags & PJ_AIRBLAST_DEFLECTABLE))
					continue;

				pProj->Deflected(pAttacker, vecDirShooting, flForce);

				bDeflected = true;
			}
		}
	}

	//only play once
	if(bDeflected)
	{
		EMIT_SOUND(ENT(pevAttacker) , CHAN_STATIC, "CKF_III/flamethrower_redirect.wav", 1.0, 0.80);
	}
}

void CBaseMonster::RadiusDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType)
{
	CKFRadiusDamage(pev->origin, this->pev, pevInflictor, pevAttacker, flDamage, flDamage * 3.5, iClassIgnore, bitsDamageType, 0, 0, 0);
}

void CBaseMonster::RadiusDamage(Vector vecSrc, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int iClassIgnore, int bitsDamageType)
{
	CKFRadiusDamage(vecSrc, this->pev, pevInflictor, pevAttacker, flDamage, flDamage * 3.5, iClassIgnore, bitsDamageType, 0, 0, 0);
}

CBaseEntity *CBaseMonster::CheckTraceHullAttack(float flDist, int iDamage, int iDmgType)
{
	if (IsPlayer())
		UTIL_MakeVectors(pev->angles);
	else
		UTIL_MakeAimVectors(pev->angles);

	Vector vecStart = pev->origin;
	vecStart.z += pev->size.z * 0.5;
	Vector vecEnd = vecStart + (gpGlobals->v_forward * flDist);

	TraceResult tr;
	UTIL_TraceHull(vecStart, vecEnd, dont_ignore_monsters, head_hull, ENT(pev), &tr);

	if (tr.pHit)
	{
		CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);

		if (iDamage > 0)
			pEntity->TakeDamage(pev, pev, iDamage, iDmgType, 0);

		return pEntity;
	}

	return NULL;
}

BOOL CBaseMonster::FInViewCone(CBaseEntity *pEntity)
{
	UTIL_MakeVectors(pev->angles);

	Vector2D vec2LOS = (pEntity->pev->origin - pev->origin).Make2D();
	vec2LOS = vec2LOS.Normalize();
	float flDot = DotProduct(vec2LOS, gpGlobals->v_forward.Make2D());

	if (flDot > m_flFieldOfView)
		return TRUE;

	return FALSE;
}

BOOL CBaseMonster::FInViewCone(Vector *pOrigin)
{
	UTIL_MakeVectors(pev->angles);

	Vector2D vec2LOS = (*pOrigin - pev->origin).Make2D();
	vec2LOS = vec2LOS.Normalize();
	float flDot = DotProduct(vec2LOS, gpGlobals->v_forward.Make2D());

	if (flDot > m_flFieldOfView)
		return TRUE;

	return FALSE;
}

BOOL CBaseEntity::FVisible(CBaseEntity *pEntity)
{
	if (FBitSet(pEntity->pev->flags, FL_NOTARGET))
		return FALSE;

	if ((pev->waterlevel != 3 && pEntity->pev->waterlevel == 3) || (pev->waterlevel == 3 && !pEntity->pev->waterlevel))
		return FALSE;

	TraceResult tr;
	Vector vecLookerOrigin = pev->origin + pev->view_ofs;
	Vector vecTargetOrigin = pEntity->EyePosition();
	UTIL_TraceLine(vecLookerOrigin, vecTargetOrigin, ignore_monsters, ignore_glass, ENT(pev), &tr);

	if (tr.flFraction != 1)
		return FALSE;

	return TRUE;
}

BOOL CBaseEntity::FVisible(const Vector &vecOrigin)
{
	TraceResult tr;
	Vector vecLookerOrigin = EyePosition();
	UTIL_TraceLine(vecLookerOrigin, vecOrigin, ignore_monsters, ignore_glass, ENT(pev), &tr);

	if (tr.flFraction != 1)
		return FALSE;

	return TRUE;
}

void CBaseEntity::TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType, int iCrit)
{
	Vector vecOrigin = ptr->vecEndPos - vecDir * 4;

	if (!pev->takedamage)
		return;

	AddMultiDamage(pevAttacker, this, flDamage, bitsDamageType, iCrit);
	int blood = BloodColor();

	if (blood != DONT_BLEED)
	{
		SpawnBlood(vecOrigin, blood, flDamage);
		TraceBleed(flDamage, vecDir, ptr, bitsDamageType);
	}
}

void CBaseMonster::TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType, int iCrit)
{
	if (!pev->takedamage)
		return;

	m_LastHitGroup = ptr->iHitgroup;

	switch (ptr->iHitgroup)
	{
		case HITGROUP_GENERIC: break;
		case HITGROUP_HEAD: flDamage *= 3; break;
		case HITGROUP_CHEST:
		case HITGROUP_STOMACH: flDamage *= 1.5; break;
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM: flDamage *= 1.0; break;
		case HITGROUP_LEFTLEG:
		case HITGROUP_RIGHTLEG: flDamage *= 0.75; break;
		default: break;
	}

	CBaseEntity::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType, iCrit);
}

void CBaseEntity::DrawAttachTracer(int iAttachment, Vector vecDst, int iColor)
{
	MESSAGE_BEGIN(MSG_BROADCAST, gmsgDrawFX);
	WRITE_BYTE(FX_ATTACHTRACER);
	WRITE_SHORT(entindex());
	WRITE_BYTE(iAttachment);
	WRITE_COORD(vecDst.x);
	WRITE_COORD(vecDst.y);
	WRITE_COORD(vecDst.z);
	WRITE_BYTE(iColor);	
	MESSAGE_END();
}

void CBaseEntity::DrawCoordTracer(Vector vecSrc, Vector vecDst, int iColor)
{
	MESSAGE_BEGIN(MSG_BROADCAST, gmsgDrawFX);
	WRITE_BYTE(FX_COORDTRACER);
	WRITE_COORD(vecSrc.x);
	WRITE_COORD(vecSrc.y);
	WRITE_COORD(vecSrc.z);	
	WRITE_COORD(vecDst.x);
	WRITE_COORD(vecDst.y);
	WRITE_COORD(vecDst.z);
	WRITE_BYTE(iColor);
	MESSAGE_END();
}

Vector CBaseEntity::CKFFireBullets(Vector vecSrc, Vector vecDirShooting, float flSpread, float flDistance, int iBulletType, int iDamage, int iCrit, entvars_t *pevAttacker, int shared_rand, BOOL bApplyDamage)
{
	Vector vecRight = gpGlobals->v_right;
	Vector vecUp = gpGlobals->v_up;
	BOOL bShowSparks = TRUE;
	int iCurrentDamage = iDamage;
	float flCurrentDistance;
	float flDamageModifier = 1.0;

	if (!pevAttacker)
		pevAttacker = pev;

	if(bApplyDamage)
		ClearMultiDamage();

	float x, y, z;

	if (IsPlayer())
	{
		x = UTIL_SharedRandomFloat(shared_rand, -0.5, 0.5) + UTIL_SharedRandomFloat(shared_rand + 1, -0.5, 0.5);
		y = UTIL_SharedRandomFloat(shared_rand + 2, -0.5, 0.5) + UTIL_SharedRandomFloat(shared_rand + 3, -0.5, 0.5);
	}
	else
	{
		do
		{
			x = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
			y = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
			z = x * x + y * y;
		}
		while (z > 1);
	}

	Vector vecDir = vecDirShooting + x * flSpread * vecRight + y * flSpread * vecUp;
	Vector vecEnd = vecSrc + vecDir * flDistance;

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(pev), &tr);

	if (tr.flFraction != 1)
	{
		CBaseEntity *pEntity = CBaseEntity::Instance(tr.pHit);
		flCurrentDistance = tr.flFraction * flDistance;

		switch (iBulletType)
		{
		case BULLET_PLAYER_SNIPER: case BULLET_PLAYER_SNIPER_NOHS:
			{
				iCurrentDamage = iDamage + (int)RANDOM_FLOAT(-iDamage*0.14, iDamage*0.14);
				if(tr.iHitgroup == HITGROUP_HEAD && iBulletType != BULLET_PLAYER_SNIPER_NOHS)
					iCrit += 2;
				break;
			}
			case BULLET_PLAYER_TF2: case BULLET_SENTRY_TF2:
			{
				if(flCurrentDistance < DIST_MEDIUM)
					flDamageModifier = -0.2*flCurrentDistance/DIST_MEDIUM+1.2;
				else if(flCurrentDistance < DIST_LONG)
					flDamageModifier = sin(M_PI/2*flCurrentDistance/DIST_LONG + M_PI/2) + 0.5;
				else
					flDamageModifier = 0.5;
				if(iCrit == 1 && flCurrentDistance >= DIST_MEDIUM)
					flDamageModifier = 1.0;
				break;
			}
			default:
			{
				iCurrentDamage = iDamage + (int)RANDOM_FLOAT(-iDamage*0.14, iDamage*0.14);
				break;
			}
		}

		if(pEntity->IsPlayer() && pEntity->IsAlive())
		{
			iCrit += pEntity->GetCriticalHit();
			if(iCrit >= 2)
				flDamageModifier = 3;
			else if(iCrit == 1)
				flDamageModifier *= 1.35;

			if(iBulletType == BULLET_SENTRY_TF2)
			{
				CBaseEntity *pAttacker = CBaseEntity::Instance(pevAttacker);
				if(pAttacker->IsPlayer())
				{
					CBasePlayer *pPlayerAttacker = (CBasePlayer *)pAttacker;
					CBasePlayer *pPlayerVictim = (CBasePlayer *)pEntity;
					if(pPlayerAttacker->m_iTeam != pPlayerVictim->m_iTeam || pEntity->pev == pevAttacker)
					{
						pEntity->KnockBack(vecDir, 180);
					}
				}
			}
		}

		iCurrentDamage = int(iDamage * flDamageModifier);
#ifndef CLIENT_WEAPONS
		DecalGunshot(&tr, iBulletType, FALSE, pev, bShowSparks);
#endif
		pEntity->TraceAttack(pevAttacker, iCurrentDamage, vecDir, &tr, DMG_BULLET | DMG_NEVERGIB, iCrit);
	}

	if(bApplyDamage)
	{
		gMultiDamage.type = DMG_BULLET | DMG_NEVERGIB;
		ApplyMultiDamage(pev, pevAttacker);
	}

	return tr.vecEndPos;
}

void CBaseEntity::TraceBleed(float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType)
{
	if (BloodColor() == DONT_BLEED)
		return;

	if (!flDamage)
		return;

	if (!(bitsDamageType & (DMG_CRUSH | DMG_BULLET | DMG_SLASH | DMG_BLAST | DMG_CLUB)))
		return;

	float flNoise;
	int cCount;

	if (flDamage < 10)
	{
		flNoise = 0.1;
		cCount = 1;
	}
	else if (flDamage < 25)
	{
		flNoise = 0.2;
		cCount = 2;
	}
	else
	{
		flNoise = 0.3;
		cCount = 4;
	}

	for (int i = 0; i < cCount; i++)
	{
		Vector vecTraceDir = vecDir * -1;
		vecTraceDir.x += RANDOM_FLOAT(-flNoise, flNoise);
		vecTraceDir.y += RANDOM_FLOAT(-flNoise, flNoise);
		vecTraceDir.z += RANDOM_FLOAT(-flNoise, flNoise);

		TraceResult Bloodtr;
		UTIL_TraceLine(ptr->vecEndPos, ptr->vecEndPos + vecTraceDir * -172, ignore_monsters, ENT(pev), &Bloodtr);

		if (Bloodtr.flFraction != 1)
		{
			if (!RANDOM_LONG(0, 2))
				UTIL_BloodDecalTrace(&Bloodtr, BloodColor());
		}
	}
}

void CBaseMonster::BloodSplat(const Vector &vecPos, const Vector &vecDir, int hitgroup, int iDamage)
{
	if (hitgroup != HITGROUP_HEAD)
		return;

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecPos);
	WRITE_BYTE(TE_BLOODSTREAM);
	WRITE_COORD(vecPos.x);
	WRITE_COORD(vecPos.y);
	WRITE_COORD(vecPos.z);
	WRITE_COORD(vecDir.x);
	WRITE_COORD(vecDir.y);
	WRITE_COORD(vecDir.z);
	WRITE_BYTE(223);
	WRITE_BYTE(iDamage + RANDOM_LONG(0, 100));
	MESSAGE_END();
}

void FindHullIntersection(const Vector &vecSrc, TraceResult &tr, float *pflMins, float *pfkMaxs, edict_t *pEntity)
{
	TraceResult trTemp;
	float flDistance = 1000000;
	float *pflMinMaxs[2] = { pflMins, pfkMaxs };
	Vector vecHullEnd = tr.vecEndPos;

	vecHullEnd = vecSrc + ((vecHullEnd - vecSrc) * 2);
	TRACE_LINE(vecSrc, vecHullEnd, dont_ignore_monsters, pEntity, &trTemp);

	if (trTemp.flFraction < 1)
	{
		tr = trTemp;
		return;
	}

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			for (int k = 0; k < 2; k++)
			{
				Vector vecEnd;
				vecEnd.x = vecHullEnd.x + pflMinMaxs[i][0];
				vecEnd.y = vecHullEnd.y + pflMinMaxs[j][1];
				vecEnd.z = vecHullEnd.z + pflMinMaxs[k][2];

				TRACE_LINE(vecSrc, vecEnd, dont_ignore_monsters, pEntity, &trTemp);

				if (trTemp.flFraction < 1)
				{
					float flThisDistance = (trTemp.vecEndPos - vecSrc).Length();

					if (flThisDistance < flDistance)
					{
						tr = trTemp;
						flDistance = flThisDistance;
					}
				}
			}
		}
	}
}