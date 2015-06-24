#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "saverestore.h"
#include "func_break.h"
#include "decals.h"
#include "explode.h"

extern DLL_GLOBAL Vector g_vecAttackDir;

const char *CBreakable::pSpawnObjects[] =
{
	NULL,
	"item_healthbox",
	"item_ammobox",
};

void CBreakable::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "explosion"))
	{
		if (!stricmp(pkvd->szValue, "directed"))
			m_Explosion = expDirected;
		else if (!stricmp(pkvd->szValue, "random"))
			m_Explosion = expRandom;
		else
			m_Explosion = expRandom;

		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "material"))
	{
		int i = atoi(pkvd->szValue);

		if (i < 0 || i >= matLastMaterial)
			m_Material = matWood;
		else
			m_Material = (Materials)i;

		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "deadmodel"))
	{
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "shards"))
	{
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "gibmodel"))
	{
		m_iszGibModel = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "spawnobject"))
	{
		int object = atoi(pkvd->szValue);

		if (object > 0 && object < (int)(ARRAYSIZE(pSpawnObjects)))
			m_iszSpawnObject = MAKE_STRING(pSpawnObjects[object]);

		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "explodemagnitude"))
	{
		ExplosionSetMagnitude(atoi(pkvd->szValue));
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "lip"))
		pkvd->fHandled = TRUE;
	else
		CBaseDelay::KeyValue(pkvd);
}

LINK_ENTITY_TO_CLASS(func_breakable, CBreakable);

void CBreakable::Spawn(void)
{
	Precache();

	if (FBitSet(pev->spawnflags, SF_BREAK_TRIGGER_ONLY))
		pev->takedamage = DAMAGE_NO;
	else
		pev->takedamage = DAMAGE_YES;

	m_flSaveHealth = pev->health;
	pev->solid = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;
	m_angle = pev->angles.y;
	pev->angles.y = 0;

	pev->origin = Center();
	pev->oldorigin = pev->origin;

	if (m_Material == matGlass)
		pev->playerclass = 1;

	SET_MODEL(ENT(pev), STRING(pev->model));
	SetTouch(&CBreakable::BreakTouch);

	if (FBitSet(pev->spawnflags, SF_BREAK_TRIGGER_ONLY))
		SetTouch(NULL);

	if (!IsBreakable() && pev->rendermode != kRenderNormal)
		pev->flags |= FL_WORLDBRUSH;
}

void CBreakable::Restart(void)
{
	pev->solid = SOLID_BSP;
	pev->movetype = MOVETYPE_PUSH;
	pev->deadflag = DEAD_NO;

	if (FBitSet(pev->spawnflags, SF_BREAK_TRIGGER_ONLY))
		pev->takedamage = DAMAGE_NO;
	else
		pev->takedamage = DAMAGE_YES;

	pev->deadflag = DEAD_NO;
	pev->health = m_flSaveHealth;
	pev->effects &= ~EF_NODRAW;
	m_angle = pev->angles.y;
	pev->angles.y = 0;

	SET_MODEL(ENT(pev), STRING(pev->model));
	SetTouch(&CBreakable::BreakTouch);

	if (FBitSet(pev->spawnflags, SF_BREAK_TRIGGER_ONLY))
		SetTouch(NULL);

	if (!IsBreakable() && pev->rendermode != kRenderNormal)
		pev->flags |= FL_WORLDBRUSH;

	pev->origin = pev->oldorigin;
	UTIL_SetOrigin(pev, pev->origin);
}

const char *CBreakable::pSoundsWood[] =
{
	"debris/wood1.wav",
	"debris/wood2.wav",
	"debris/wood3.wav",
};

const char *CBreakable::pSoundsFlesh[] =
{
	"debris/flesh1.wav",
	"debris/flesh2.wav",
	"debris/flesh3.wav",
	"debris/flesh5.wav",
	"debris/flesh6.wav",
	"debris/flesh7.wav",
};

const char *CBreakable::pSoundsMetal[] =
{
	"debris/metal1.wav",
	"debris/metal2.wav",
	"debris/metal3.wav",
};

const char *CBreakable::pSoundsConcrete[] =
{
	"debris/concrete1.wav",
	"debris/concrete2.wav",
	"debris/concrete3.wav",
};

const char *CBreakable::pSoundsGlass[] =
{
	"debris/glass1.wav",
	"debris/glass2.wav",
	"debris/glass3.wav",
};

const char **CBreakable::MaterialSoundList(Materials precacheMaterial, int &soundCount)
{
	const char **pSoundList = NULL;

	switch (precacheMaterial)
	{
		case matWood:
		{
			pSoundList = pSoundsWood;
			soundCount = ARRAYSIZE(pSoundsWood);
			break;
		}

		case matFlesh:
		{
			pSoundList = pSoundsFlesh;
			soundCount = ARRAYSIZE(pSoundsFlesh);
			break;
		}

		case matComputer:
		case matUnbreakableGlass:
		case matGlass:
		{
			pSoundList = pSoundsGlass;
			soundCount = ARRAYSIZE(pSoundsGlass);
			break;
		}

		case matMetal:
		{
			pSoundList = pSoundsMetal;
			soundCount = ARRAYSIZE(pSoundsMetal);
			break;
		}

		case matCinderBlock:
		case matRocks:
		{
			pSoundList = pSoundsConcrete;
			soundCount = ARRAYSIZE(pSoundsConcrete);
			break;
		}

		case matCeilingTile:
		case matNone:
		default: soundCount = 0; break;
	}

	return pSoundList;
}

void CBreakable::MaterialSoundPrecache(Materials precacheMaterial)
{
	int soundCount = 0;
	const char **pSoundList = MaterialSoundList(precacheMaterial, soundCount);

	for (int i = 0; i < soundCount; i++)
		PRECACHE_SOUND((char *)pSoundList[i]);
}

void CBreakable::MaterialSoundRandom(edict_t *pEdict, Materials soundMaterial, float volume)
{
	int soundCount = 0;
	const char **pSoundList = MaterialSoundList(soundMaterial, soundCount);

	if (soundCount)
		EMIT_SOUND(pEdict, CHAN_BODY, pSoundList[RANDOM_LONG(0, soundCount - 1)], volume, 1);
}

void CBreakable::Precache(void)
{
	const char *pGibName = NULL;

	switch (m_Material)
	{
		case matWood:
		{
			pGibName = "models/woodgibs.mdl";
			PRECACHE_SOUND("debris/bustcrate1.wav");
			PRECACHE_SOUND("debris/bustcrate2.wav");
			break;
		}

		case matFlesh:
		{
			pGibName = "models/fleshgibs.mdl";
			PRECACHE_SOUND("debris/bustflesh1.wav");
			PRECACHE_SOUND("debris/bustflesh2.wav");
			break;
		}

		case matComputer:
		{
			PRECACHE_SOUND("buttons/spark5.wav");
			PRECACHE_SOUND("buttons/spark6.wav");
			pGibName = "models/computergibs.mdl";
			PRECACHE_SOUND("debris/bustmetal1.wav");
			PRECACHE_SOUND("debris/bustmetal2.wav");
			break;
		}

		case matUnbreakableGlass:
		case matGlass:
		{
			pGibName = "models/glassgibs.mdl";
			PRECACHE_SOUND("debris/bustglass1.wav");
			PRECACHE_SOUND("debris/bustglass2.wav");
			break;
		}

		case matMetal:
		{
			pGibName = "models/metalplategibs.mdl";
			PRECACHE_SOUND("debris/bustmetal1.wav");
			PRECACHE_SOUND("debris/bustmetal2.wav");
			break;
		}

		case matCinderBlock:
		{
			pGibName = "models/cindergibs.mdl";
			PRECACHE_SOUND("debris/bustconcrete1.wav");
			PRECACHE_SOUND("debris/bustconcrete2.wav");
			break;
		}

		case matRocks:
		{
			pGibName = "models/rockgibs.mdl";
			PRECACHE_SOUND("debris/bustconcrete1.wav");
			PRECACHE_SOUND("debris/bustconcrete2.wav");
			break;
		}

		case matCeilingTile:
		{
			pGibName = "models/ceilinggibs.mdl";
			PRECACHE_SOUND("debris/bustceiling.wav");
			break;
		}
	}

	MaterialSoundPrecache(m_Material);

	if (m_iszGibModel)
		pGibName = STRING(m_iszGibModel);

	m_idShard = PRECACHE_MODEL((char *)pGibName);

	if (m_iszSpawnObject)
		UTIL_PrecacheOther((char *)STRING(m_iszSpawnObject));
}

void CBreakable::DamageSound(void)
{
	char *rgpsz[6];
	int i = 0;
	int material = m_Material;

	int pitch = RANDOM_LONG(0, 2) ? PITCH_NORM : (95 + RANDOM_LONG(0, 34));
	float fvol = RANDOM_FLOAT(0.75, 1);

	if (material == matComputer && RANDOM_LONG(0, 1))
		material = matMetal;

	switch (material)
	{
		case matComputer:
		case matGlass:
		case matUnbreakableGlass:
		{
			rgpsz[0] = "debris/glass1.wav";
			rgpsz[1] = "debris/glass2.wav";
			rgpsz[2] = "debris/glass3.wav";
			i = 3;
			break;
		}

		case matWood:
		{
			rgpsz[0] = "debris/wood1.wav";
			rgpsz[1] = "debris/wood2.wav";
			rgpsz[2] = "debris/wood3.wav";
			i = 3;
			break;
		}

		case matMetal:
		{
			rgpsz[0] = "debris/metal1.wav";
			rgpsz[1] = "debris/metal3.wav";
			rgpsz[2] = "debris/metal2.wav";
			i = 2;
			break;
		}

		case matFlesh:
		{
			rgpsz[0] = "debris/flesh1.wav";
			rgpsz[1] = "debris/flesh2.wav";
			rgpsz[2] = "debris/flesh3.wav";
			rgpsz[3] = "debris/flesh5.wav";
			rgpsz[4] = "debris/flesh6.wav";
			rgpsz[5] = "debris/flesh7.wav";
			i = 6;
			break;
		}

		case matRocks:
		case matCinderBlock:
		{
			rgpsz[0] = "debris/concrete1.wav";
			rgpsz[1] = "debris/concrete2.wav";
			rgpsz[2] = "debris/concrete3.wav";
			i = 3;
			break;
		}

		case matCeilingTile:
		{
			i = 0;
			break;
		}
	}

	if (i)
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, rgpsz[RANDOM_LONG(0, i - 1)], fvol, ATTN_NORM, 0, pitch);
}

void CBreakable::BreakTouch(CBaseEntity *pOther)
{
	entvars_t *pevToucher = pOther->pev;

	if (!pOther->IsPlayer() || !IsBreakable())
	{
		if (pev->rendermode == kRenderNormal || !FClassnameIs(pOther->pev, "grenade"))
			return;

		pev->angles.y = m_angle;
		UTIL_MakeVectors(pev->angles);
		g_vecAttackDir = gpGlobals->v_forward;
		pev->takedamage = DAMAGE_NO;
		pev->deadflag = DEAD_DEAD;
		pev->effects = EF_NODRAW;
		Die();
	}

	if (FBitSet(pev->spawnflags, SF_BREAK_TOUCH))
	{
		float flDamage = pevToucher->velocity.Length() * 0.01;

		if (flDamage >= pev->health)
		{
			SetTouch(NULL);
			TakeDamage(pevToucher, pevToucher, flDamage, DMG_CRUSH, 0);
			pOther->TakeDamage(pev, pev, flDamage / 4, DMG_SLASH, 0);
		}
	}

	if (FBitSet(pev->spawnflags, SF_BREAK_PRESSURE) && pevToucher->absmin.z >= pev->maxs.z - 2)
	{
		DamageSound();
		SetThink(&CBreakable::Die);
		SetTouch(NULL);

		if (m_flDelay == 0)
			m_flDelay = 0.1;

		pev->nextthink = pev->ltime + m_flDelay;
	}
}

void CBreakable::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if(pev->deadflag = DEAD_DEAD && useType != USE_OFF)
	{
		Restart();
	}
	else if (IsBreakable())
	{
		pev->angles.y = m_angle;
		UTIL_MakeVectors(pev->angles);
		g_vecAttackDir = gpGlobals->v_forward;
		pev->takedamage = DAMAGE_NO;
		pev->deadflag = DEAD_DEAD;
		pev->effects = EF_NODRAW;
		Die();
	}
}

void CBreakable::TraceAttack(entvars_t *pevAttacker, float flDamage, Vector vecDir, TraceResult *ptr, int bitsDamageType, int iCrit)
{
	if (RANDOM_LONG(0, 1))
	{
		switch (m_Material)
		{
			case matComputer:
			{
				UTIL_Sparks(ptr->vecEndPos);
				float flVolume = RANDOM_FLOAT(0.7, 1);

				switch (RANDOM_LONG(0, 1))
				{
					case 0: EMIT_SOUND(ENT(pev), CHAN_VOICE, "buttons/spark5.wav", flVolume, ATTN_NORM); break;
					case 1: EMIT_SOUND(ENT(pev), CHAN_VOICE, "buttons/spark6.wav", flVolume, ATTN_NORM); break;
				}

				break;
			}

			case matUnbreakableGlass:
			{
				UTIL_Ricochet(ptr->vecEndPos, RANDOM_FLOAT(0.5, 1.5));
				break;
			}
		}
	}

	CBaseDelay::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType, iCrit);
}

int CBreakable::TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType, int iCrit)
{
	Vector vecTemp;

 	if (pevAttacker == pevInflictor)
	{
		vecTemp = pevInflictor->origin - (pev->absmin + (pev->size * 0.5));

		if (FBitSet(pevAttacker->flags, FL_CLIENT) && FBitSet(pev->spawnflags, SF_BREAK_CROWBAR) && (bitsDamageType & DMG_CLUB))
			flDamage = pev->health;
	}
	else
		vecTemp = pevInflictor->origin - (pev->absmin + (pev->size * 0.5));

	if (!IsBreakable())
		return 0;

	if (bitsDamageType & DMG_CLUB)
		flDamage *= 2;

	g_vecAttackDir = vecTemp.Normalize();
	pev->health -= flDamage;

	if (pev->health <= 0)
	{
		pev->takedamage = DAMAGE_NO;
		pev->deadflag = DEAD_DEAD;
		pev->effects = EF_NODRAW;
		Die();

		if (m_flDelay == 0)
			m_flDelay = 0.1;

		pev->nextthink = pev->ltime + m_flDelay;
		return 0;
	}

	DamageSound();
	return 1;
}

void CBreakable::Die(void)
{
	char cFlag = 0;
	int pitch = 95 + RANDOM_LONG(0, 29);

	if (pitch > 97 && pitch < 103)
		pitch = 100;

	float fvol = RANDOM_FLOAT(0.85, 1) + (abs((int)pev->health) / 100);

	if (fvol > 1)
		fvol = 1;

	switch (m_Material)
	{
		case matGlass:
		{
			switch (RANDOM_LONG(0, 1))
			{
				case 0: EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustglass1.wav", fvol, ATTN_NORM, 0, pitch); break;
				case 1: EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustglass2.wav", fvol, ATTN_NORM, 0, pitch); break;
			}

			cFlag = BREAK_GLASS;
			break;
		}

		case matWood:
		{
			switch (RANDOM_LONG(0, 1))
			{
				case 0: EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustcrate1.wav", fvol, ATTN_NORM, 0, pitch); break;
				case 1: EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustcrate2.wav", fvol, ATTN_NORM, 0, pitch); break;
			}

			cFlag = BREAK_WOOD;
			break;
		}

		case matComputer:
		case matMetal:
		{
			switch (RANDOM_LONG(0, 1))
			{
				case 0: EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustmetal1.wav", fvol, ATTN_NORM, 0, pitch); break;
				case 1: EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustmetal2.wav", fvol, ATTN_NORM, 0, pitch); break;
			}

			cFlag = BREAK_METAL;
			break;
		}

		case matFlesh:
		{
			switch (RANDOM_LONG(0, 1))
			{
				case 0: EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustflesh1.wav", fvol, ATTN_NORM, 0, pitch); break;
				case 1: EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustflesh2.wav", fvol, ATTN_NORM, 0, pitch); break;
			}

			cFlag = BREAK_FLESH;
			break;
		}

		case matRocks:
		case matCinderBlock:
		{
			switch (RANDOM_LONG(0, 1))
			{
				case 0: EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustconcrete1.wav", fvol, ATTN_NORM, 0, pitch); break;
				case 1: EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustconcrete2.wav", fvol, ATTN_NORM, 0, pitch); break;
			}

			cFlag = BREAK_CONCRETE;
			break;
		}

		case matCeilingTile: EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, "debris/bustceiling.wav", fvol, ATTN_NORM, 0, pitch); break;
	}

	Vector vecSpot, vecVelocity;

	if (m_Explosion == expDirected)
	{
		vecVelocity = g_vecAttackDir * 200;
	}
	else
	{
		vecVelocity.x = 0;
		vecVelocity.y = 0;
		vecVelocity.z = 0;
	}

	vecSpot = pev->origin + (pev->mins + pev->maxs) * 0.5;

	MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, vecSpot);
	WRITE_BYTE(TE_BREAKMODEL);
	WRITE_COORD(vecSpot.x);
	WRITE_COORD(vecSpot.y);
	WRITE_COORD(vecSpot.z);
	WRITE_COORD(pev->size.x);
	WRITE_COORD(pev->size.y);
	WRITE_COORD(pev->size.z);
	WRITE_COORD(vecVelocity.x);
	WRITE_COORD(vecVelocity.y);
	WRITE_COORD(vecVelocity.z);
	WRITE_BYTE(10);
	WRITE_SHORT(m_idShard);
	WRITE_BYTE(0);
	WRITE_BYTE(25);
	WRITE_BYTE(cFlag);
	MESSAGE_END();

	float size = pev->size.x;

	if (size < pev->size.y)
		size = pev->size.y;

	if (size < pev->size.z)
		size = pev->size.z;

	Vector mins = pev->absmin;
	Vector maxs = pev->absmax;
	mins.z = pev->absmax.z;
	maxs.z += 8;

	CBaseEntity *pList[256];
	int count = UTIL_EntitiesInBox(pList, 256, mins, maxs, FL_ONGROUND);

	if (count)
	{
		for (int i = 0; i < count; i++)
		{
			ClearBits(pList[i]->pev->flags, FL_ONGROUND);
			pList[i]->pev->groundentity = NULL;
		}
	}

	pev->solid = SOLID_NOT;
	SUB_UseTargets(NULL, USE_TOGGLE, 0);
	SetThink(NULL);
	pev->nextthink = pev->ltime + 0.1;

	if (m_iszSpawnObject)
		CBaseEntity::Create((char *)STRING(m_iszSpawnObject), VecBModelOrigin(pev), pev->angles, edict());

	if (Explodable())
		ExplosionCreate(Center(), pev->angles, edict(), ExplosionMagnitude(), TRUE);
}

BOOL CBreakable::IsBreakable(void)
{
	return m_Material != matUnbreakableGlass;
}

int CBreakable::DamageDecal(int bitsDamageType)
{
	if (m_Material == matGlass)
		return DECAL_GLASSBREAK1 + RANDOM_LONG(0, 2);

	if (m_Material == matUnbreakableGlass)
		return DECAL_BPROOF1;

	return CBaseEntity::DamageDecal(bitsDamageType);
}

class CPushable : public CBreakable
{
public:
	void Spawn(void);
	void Precache(void);
	void Touch (CBaseEntity *pOther);
	void Move(CBaseEntity *pMover, int push);
	void KeyValue(KeyValueData *pkvd);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	int ObjectCaps(void) { return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_CONTINUOUS_USE; }
	int TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType, int iCrit);
	void Restart(void);
public:
	void EXPORT StopSound(void);
	inline float MaxSpeed(void) { return m_maxSpeed; }

public:
	static char *m_soundNames[3];
	int m_lastSound;
	float m_maxSpeed;
	float m_soundTime;
};

LINK_ENTITY_TO_CLASS(func_pushable, CPushable);

char *CPushable::m_soundNames[3] = { "debris/pushbox1.wav", "debris/pushbox2.wav", "debris/pushbox3.wav" };

void CPushable::Spawn(void)
{
	if (pev->spawnflags & SF_PUSH_BREAKABLE)
		CBreakable::Spawn();
	else
		Precache();

	pev->movetype = MOVETYPE_PUSHSTEP;
	pev->solid = SOLID_BBOX;
	SET_MODEL(ENT(pev), STRING(pev->model));

	if (pev->friction > 399)
		pev->friction = 399;

	m_maxSpeed = 400 - pev->friction;
	SetBits(pev->flags, FL_FLOAT);
	pev->friction = 0;
	pev->origin.z += 1;
	UTIL_SetOrigin(pev, pev->origin);
	pev->oldorigin = pev->origin;

	pev->skin = (int)((pev->skin * (pev->maxs.x - pev->mins.x) * (pev->maxs.y - pev->mins.y)) * 0.0005);
	m_soundTime = 0;
}

void CPushable::Precache(void)
{
	for (int i = 0; i < 3; i++)
		PRECACHE_SOUND(m_soundNames[i]);

	if (pev->spawnflags & SF_PUSH_BREAKABLE)
		CBreakable::Precache();
}

void CPushable::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "size"))
	{
		int bbox = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;

		switch (bbox)
		{
			case 0: UTIL_SetSize(pev, Vector(-8, -8, -8), Vector(8, 8, 8)); break;
			case 2: UTIL_SetSize(pev, VEC_DUCK_HULL_MIN * 2, VEC_DUCK_HULL_MAX * 2); break;
			case 3: UTIL_SetSize(pev, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX); break;

			default:
			case 1: UTIL_SetSize(pev, VEC_HULL_MIN, VEC_HULL_MAX); break;
		}
	}
	else if (FStrEq(pkvd->szKeyName, "buoyancy"))
	{
		pev->skin = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBreakable::KeyValue(pkvd);
}

void CPushable::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (!pActivator || !pActivator->IsPlayer())
	{
		if (pev->spawnflags & SF_PUSH_BREAKABLE)
			this->CBreakable::Use(pActivator, pCaller, useType, value);

		return;
	}

	if (pActivator->pev->velocity != g_vecZero)
		Move(pActivator, 0);
}

void CPushable::Touch(CBaseEntity *pOther)
{
	if (FClassnameIs(pOther->pev, "worldspawn"))
		return;

	Move(pOther, 1);
}

void CPushable::Move(CBaseEntity *pOther, int push)
{
	entvars_t *pevToucher = pOther->pev;
	int playerTouch = 0;

	if (FBitSet(pevToucher->flags, FL_ONGROUND) && pevToucher->groundentity && VARS(pevToucher->groundentity) == pev)
	{
		if (pev->waterlevel > 0)
			pev->velocity.z += pevToucher->velocity.z * 0.1;

		return;
	}

	if (pOther->IsPlayer())
	{
		if (push && !(pevToucher->button & (IN_FORWARD | IN_USE)))
			return;

		playerTouch = 1;
	}

	float factor;

	if (playerTouch)
	{
		if (!(pevToucher->flags & FL_ONGROUND))
		{
			if (pev->waterlevel < 1)
				return;
			else
				factor = 0.1;
		}
		else
			factor = 1;
	}
	else
		factor = 0.25;

	/*

	pev->velocity.x += pevToucher->velocity.x * factor;
	pev->velocity.y += pevToucher->velocity.y * factor;
	float length = sqrt(pev->velocity.x * pev->velocity.x + pev->velocity.y * pev->velocity.y);

	if (push && (length > MaxSpeed()))
	{
		pev->velocity.x = (pev->velocity.x * MaxSpeed() / length);
		pev->velocity.y = (pev->velocity.y * MaxSpeed() / length);
	}

	if (playerTouch)
	{
		//hz fix??
		//pevToucher->velocity.x = pev->velocity.x;
		//pevToucher->velocity.y = pev->velocity.y;

		if ((gpGlobals->time - m_soundTime) > 0.7)
		{
			m_soundTime = gpGlobals->time;

			if (length > 0 && FBitSet(pev->flags, FL_ONGROUND))
			{
				m_lastSound = RANDOM_LONG(0, 2);
				EMIT_SOUND(ENT(pev), CHAN_WEAPON, m_soundNames[m_lastSound], 0.5, ATTN_NORM);
			}
			else
				STOP_SOUND(ENT(pev), CHAN_WEAPON, m_soundNames[m_lastSound]);
		}
	}
	*/

	//sohl code

	if (!push)
	factor = factor*0.5;

	Vector oldVelocity = pev->velocity;
	pev->velocity.x += pevToucher->velocity.x * factor;
	pev->velocity.y += pevToucher->velocity.y * factor;

	float length = sqrt( pev->velocity.x * pev->velocity.x + pev->velocity.y * pev->velocity.y );
	if ( push && (length > MaxSpeed()) )
	{
		pev->velocity.x = (pev->velocity.x * MaxSpeed() / length );
		pev->velocity.y = (pev->velocity.y * MaxSpeed() / length );
	}

	if ( playerTouch )
	{
		float playerSpeed = pevToucher->velocity.Length2D();
		Vector playerPushDir = pevToucher->velocity/playerSpeed;
		playerPushDir.z = 0;
		float newdot = DotProduct( playerPushDir, pev->velocity );
		float olddot = DotProduct( playerPushDir, oldVelocity );
		if ( newdot > playerSpeed )
		{
			pev->velocity.x -= playerPushDir.x * newdot;
			pev->velocity.y -= playerPushDir.y * newdot;
			pev->velocity.x += pevToucher->velocity.x;
			pev->velocity.y += pevToucher->velocity.y;
		}
		//pevToucher->velocity.x = pev->velocity.x;
		//pevToucher->velocity.y = pev->velocity.y;
		if ( (gpGlobals->time - m_soundTime) > 0.7 )
		{
			m_soundTime = gpGlobals->time;
			if ( length > 0 && FBitSet(pev->flags,FL_ONGROUND) )
			{
				m_lastSound = RANDOM_LONG(0,2);
				EMIT_SOUND(ENT(pev), CHAN_WEAPON, m_soundNames[m_lastSound], 0.5, ATTN_NORM);
			}
			else
				STOP_SOUND( ENT(pev), CHAN_WEAPON, m_soundNames[m_lastSound] );
		}
	}
}

int CPushable::TakeDamage(entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamageType, int iCrit)
{
	if (pev->spawnflags & SF_PUSH_BREAKABLE)
		return CBreakable::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType, iCrit);

	return 1;
}

void CPushable::Restart(void)
{
	pev->movetype = MOVETYPE_PUSHSTEP;
	pev->solid = SOLID_BBOX;
	pev->deadflag = DEAD_NO;

	if (FBitSet(pev->spawnflags, SF_BREAK_TRIGGER_ONLY))
		pev->takedamage = DAMAGE_NO;
	else
		pev->takedamage = DAMAGE_YES;

	pev->deadflag = DEAD_NO;
	pev->health = m_flSaveHealth;
	pev->effects &= ~EF_NODRAW;
	m_angle = pev->angles.y;
	pev->angles.y = 0;

	SET_MODEL(ENT(pev), STRING(pev->model));
	SetTouch(&CBreakable::BreakTouch);

	if (FBitSet(pev->spawnflags, SF_BREAK_TRIGGER_ONLY))
		SetTouch(NULL);

	if (!IsBreakable() && pev->rendermode != kRenderNormal)
		pev->flags |= FL_WORLDBRUSH;

	//sohl PostSpawn code
	pev->origin = pev->oldorigin;
	UTIL_SetOrigin(pev, pev->origin);
}