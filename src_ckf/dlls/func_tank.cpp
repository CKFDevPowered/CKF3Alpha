#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "effects.h"
#include "weapons.h"
#include "explode.h"
#include "player.h"

#define SF_TANK_ACTIVE 0x0001
#define SF_TANK_PLAYER 0x0002
#define SF_TANK_HUMANS 0x0004
#define SF_TANK_ALIENS 0x0008
#define SF_TANK_LINEOFSIGHT 0x0010
#define SF_TANK_CANCONTROL 0x0020
#define SF_TANK_SOUNDON 0x8000

enum TANKBULLET
{
	TANK_BULLET_NONE = 0,
	TANK_BULLET_9MM = 1,
	TANK_BULLET_MP5 = 2,
	TANK_BULLET_12MM = 3,
};

class CFuncTank : public CBaseEntity
{
public:
	virtual void Spawn(void);
	virtual void PostSpawn(void);
	virtual void Restart(void);
	virtual void Precache(void);
	virtual void KeyValue(KeyValueData *pkvd);
	virtual void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	virtual void Think(void);
	virtual void Fire(const Vector &barrelEnd, const Vector &forward, entvars_t *pevAttacker);
	virtual void TryFire( const Vector &barrelEnd, const Vector &forward, entvars_t *pevAttacker );//sohl
	virtual Vector UpdateTargetPosition(CBaseEntity *pTarget) { return pTarget->BodyTarget(pev->origin); }
	virtual int ObjectCaps(void) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

public:
	void TrackTarget(void);
	void StartRotSound(void);
	void StopRotSound(void);

	BOOL InRange(float range);
	edict_t *FindTarget(edict_t *pPlayer);
	void TankTrace(const Vector &vecStart, const Vector &vecForward, const float flSpread, TraceResult &tr);

	Vector BarrelPosition(void)
	{
		Vector forward, right, up;
		UTIL_MakeVectorsPrivate(pev->angles, forward, right, up);
		return pev->origin + (forward * m_barrelPos.x) + (right * m_barrelPos.y) + (up * m_barrelPos.z);
	}

	void AdjustAnglesForBarrel(Vector &angles, float distance);
	BOOL OnControls(entvars_t *pevTest);
	BOOL StartControl(CBasePlayer *pController);
	void StopControl(void);
	void ControllerPostFrame(void);

public:
	inline BOOL IsActive(void) { return (pev->spawnflags & SF_TANK_ACTIVE)?TRUE:FALSE; }
	inline void TankActivate(void) { pev->spawnflags |= SF_TANK_ACTIVE; pev->nextthink = pev->ltime + 0.1; m_fireLast = 0; }
	inline void TankDeactivate(void) { pev->spawnflags &= ~SF_TANK_ACTIVE; m_fireLast = 0; StopRotSound(); }
	inline BOOL CanFire(void) { return (gpGlobals->time - m_lastSightTime) < m_persist; }

public:
	static TYPEDESCRIPTION m_SaveData[];

protected:
	CBasePlayer *m_pController;
	float m_flNextAttack;
	Vector m_vecControllerUsePos;
	float m_yawCenter;
	float m_yawRate;
	float m_yawRange;
	float m_yawTolerance;
	float m_pitchCenter;
	float m_pitchRate;
	float m_pitchRange;
	float m_pitchTolerance;
	float m_fireLast;
	float m_fireRate;
	float m_lastSightTime;
	float m_persist;
	float m_minRange;
	float m_maxRange;
	Vector m_barrelPos;
	float m_spriteScale;
	int m_iszSpriteSmoke;
	int m_iszSpriteFlash;
	TANKBULLET m_bulletType;
	int m_iBulletDamage;
	Vector m_sightOrigin;
	int m_iszMaster;
	int	m_iszFireMaster;//hz added
	float m_flTankSpread;//hz added
};

void CFuncTank::Spawn(void)
{
	Precache();

	pev->movetype = MOVETYPE_PUSH;
	pev->solid = SOLID_BSP;
	SET_MODEL(ENT(pev), STRING(pev->model));

	m_yawCenter = pev->angles.y;
	m_pitchCenter = pev->angles.x;

	if (IsActive())
		pev->nextthink = pev->ltime + 1;

	if((m_maxRange == 0) || (FStringNull(m_maxRange))) //sohl
	{
		m_maxRange = 4096;
	}

	m_sightOrigin = BarrelPosition();

	if (m_fireRate <= 0)
		m_fireRate = 1;

	pev->oldorigin = pev->origin;
}

void CFuncTank::Restart(void)
{
	pev->angles.y = m_yawCenter;
	pev->angles.x = m_pitchCenter;
}

void CFuncTank::PostSpawn(void)
{
	m_yawCenter = pev->angles.y;
	m_pitchCenter = pev->angles.x;
	return CBaseEntity::PostSpawn();
}

void CFuncTank::Precache(void)
{
	if (m_iszSpriteSmoke)
		PRECACHE_MODEL((char *)STRING(m_iszSpriteSmoke));

	if (m_iszSpriteFlash)
		PRECACHE_MODEL((char *)STRING(m_iszSpriteFlash));

	if (pev->noise)
		PRECACHE_SOUND((char *)STRING(pev->noise));
}

void CFuncTank::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "yawrate"))
	{
		m_yawRate = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "yawrange"))
	{
		m_yawRange = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "yawtolerance"))
	{
		m_yawTolerance = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "pitchrange"))
	{
		m_pitchRange = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "pitchrate"))
	{
		m_pitchRate = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "pitchtolerance"))
	{
		m_pitchTolerance = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "firerate"))
	{
		m_fireRate = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "barrel"))
	{
		m_barrelPos.x = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "barrely"))
	{
		m_barrelPos.y = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "barrelz"))
	{
		m_barrelPos.z = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "spritescale"))
	{
		m_spriteScale = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "spritesmoke"))
	{
		m_iszSpriteSmoke = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "spriteflash"))
	{
		m_iszSpriteFlash = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "rotatesound"))
	{
		pev->noise = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "persistence"))
	{
		m_persist = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "bullet"))
	{
		m_bulletType = (TANKBULLET)atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "bullet_damage"))
	{
		m_iBulletDamage = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "firespread"))//hz added this
	{
		m_flTankSpread = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "firemaster"))//hz added this
	{
		m_iszFireMaster = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "minRange"))
	{
		m_minRange = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "maxRange"))
	{
		m_maxRange = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "master"))
	{
		m_iszMaster = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

BOOL CFuncTank::OnControls(entvars_t *pevTest)
{
	if (!(pev->spawnflags & SF_TANK_CANCONTROL))
		return FALSE;

	Vector offset = pevTest->origin - pev->origin;

	if ((m_vecControllerUsePos - pevTest->origin).Length() < 30)
		return TRUE;

	return FALSE;
}

BOOL CFuncTank::StartControl(CBasePlayer *pController)
{
	if (m_pController != NULL)
		return FALSE;

	if (m_iszMaster && !UTIL_IsMasterTriggered(m_iszMaster, pController))
		return FALSE;

	ALERT(at_console, "using TANK!\n");
	m_pController = pController;

	if (m_pController->m_pActiveItem)
	{
		m_pController->m_pActiveItem->Holster();
		m_pController->pev->weaponmodel = 0;
	}

	m_pController->m_iHideHUD |= HIDEHUD_WEAPONS;
	m_vecControllerUsePos = m_pController->pev->origin;
	pev->nextthink = pev->ltime + 0.1;
	return TRUE;
}

void CFuncTank::StopControl(void)
{
	if (!m_pController)
		return;

	if (m_pController->m_pActiveItem)
		m_pController->m_pActiveItem->Deploy();

	ALERT(at_console, "stopped using TANK\n");
	m_pController->m_iHideHUD &= ~HIDEHUD_WEAPONS;

	pev->nextthink = 0;
	m_pController = NULL;

	if (IsActive())
		pev->nextthink = pev->ltime + 1;
}

void CFuncTank::ControllerPostFrame(void)
{
	if (gpGlobals->time < m_flNextAttack)
		return;

	if (m_pController->pev->button & IN_ATTACK)
	{
		Vector vecForward;
		UTIL_MakeVectorsPrivate(pev->angles, vecForward, NULL, NULL);

		m_fireLast = gpGlobals->time - (1 / m_fireRate) - 0.01;
		TryFire(BarrelPosition(), vecForward, m_pController->pev);

		if (m_pController && m_pController->IsPlayer())
			((CBasePlayer *)m_pController)->m_iWeaponVolume = LOUD_GUN_VOLUME;

		m_flNextAttack = gpGlobals->time + (1 / m_fireRate);
	}
}

void CFuncTank::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (pev->spawnflags & SF_TANK_CANCONTROL)
	{
		if (pActivator->Classify() != CLASS_PLAYER)
			return;

		if (value == 2 && useType == USE_SET)
		{
			ControllerPostFrame();
		}
		else if (!m_pController && useType != USE_OFF)
		{
			((CBasePlayer*)pActivator)->m_pTank = this;
			StartControl((CBasePlayer*)pActivator);
		}
		else
			StopControl();
	}
	else
	{
		if (!ShouldToggle(useType, IsActive()))
			return;

		if (IsActive())
			TankDeactivate();
		else
			TankActivate();
	}
}

edict_t *CFuncTank::FindTarget(edict_t *pPlayer)
{
	return pPlayer;
}

BOOL CFuncTank::InRange(float range)
{
	if (range < m_minRange)
		return FALSE;

	if (m_maxRange > 0 && range > m_maxRange)
		return FALSE;

	return TRUE;
}

void CFuncTank::Think(void)
{
	pev->avelocity = g_vecZero;
	TrackTarget();

	if (fabs(pev->avelocity.x) > 1 || fabs(pev->avelocity.y) > 1)
		StartRotSound();
	else
		StopRotSound();
}

void CFuncTank::TrackTarget(void)
{
	TraceResult tr;
	edict_t *pPlayer = FIND_CLIENT_IN_PVS(edict());
	BOOL updateTime = FALSE, lineOfSight;
	Vector angles, direction, targetPosition, barrelEnd;
	edict_t *pTarget = NULL;

	if (m_pController)
	{
		angles = m_pController->pev->v_angle;
		angles[0] = 0 - angles[0];
		pev->nextthink = pev->ltime + 0.05;
	}
	else
	{
		if (IsActive())
			pev->nextthink = pev->ltime + 0.1;
		else
			return;

		if (FNullEnt(pPlayer))
		{
			if (IsActive())
				pev->nextthink = pev->ltime + 2;

			return;
		}

		pTarget = FindTarget(pPlayer);

		if (!pTarget)
			return;

		barrelEnd = BarrelPosition();
		targetPosition = pTarget->v.origin + pTarget->v.view_ofs;
		float range = (targetPosition - barrelEnd).Length();

		if (!InRange(range))
			return;

		UTIL_TraceLine(barrelEnd, targetPosition, dont_ignore_monsters, edict(), &tr);
		lineOfSight = FALSE;

		if (tr.flFraction == 1 || tr.pHit == pTarget)
		{
			lineOfSight = TRUE;
			CBaseEntity *pInstance = CBaseEntity::Instance(pTarget);

			if (InRange(range) && pInstance && pInstance->IsAlive())
			{
				updateTime = TRUE;
				m_sightOrigin = UpdateTargetPosition(pInstance);
			}
		}

		direction = m_sightOrigin - pev->origin;
		angles = UTIL_VecToAngles(direction);
		AdjustAnglesForBarrel(angles, direction.Length());
	}

	angles.x = -angles.x;
	angles.y = m_yawCenter + UTIL_AngleDistance(angles.y, m_yawCenter);
	angles.x = m_pitchCenter + UTIL_AngleDistance(angles.x, m_pitchCenter);

	if (angles.y > m_yawCenter + m_yawRange)
	{
		angles.y = m_yawCenter + m_yawRange;
		updateTime = FALSE;
	}
	else if (angles.y < (m_yawCenter - m_yawRange))
	{
		angles.y = (m_yawCenter - m_yawRange);
		updateTime = FALSE;
	}

	if (updateTime)
		m_lastSightTime = gpGlobals->time;

	float distY = UTIL_AngleDistance(angles.y, pev->angles.y);
	pev->avelocity.y = distY * 10;

	if (pev->avelocity.y > m_yawRate)
		pev->avelocity.y = m_yawRate;

	else if (pev->avelocity.y < -m_yawRate)
		pev->avelocity.y = -m_yawRate;

	if (angles.x > m_pitchCenter + m_pitchRange)
		angles.x = m_pitchCenter + m_pitchRange;
	else if (angles.x < m_pitchCenter - m_pitchRange)
		angles.x = m_pitchCenter - m_pitchRange;

	float distX = UTIL_AngleDistance(angles.x, pev->angles.x);
	pev->avelocity.x = distX * 10;

	if (pev->avelocity.x > m_pitchRate)
		pev->avelocity.x = m_pitchRate;
	else if (pev->avelocity.x < -m_pitchRate)
		pev->avelocity.x = -m_pitchRate;

	if (m_pController)
		return;

	if (CanFire() && ((fabs(distX) < m_pitchTolerance && fabs(distY) < m_yawTolerance) || (pev->spawnflags & SF_TANK_LINEOFSIGHT)))
	{
		BOOL fire = FALSE;
		Vector forward;
		UTIL_MakeVectorsPrivate(pev->angles, forward, NULL, NULL);

		if (pev->spawnflags & SF_TANK_LINEOFSIGHT)
		{
			float length = direction.Length();
			UTIL_TraceLine(barrelEnd, barrelEnd + forward * length, dont_ignore_monsters, edict(), &tr);

			if (tr.pHit == pTarget)
				fire = TRUE;
		}
		else
			fire = TRUE;

		if (fire)
			TryFire(BarrelPosition(), forward, pev);
		else
			m_fireLast = 0;
	}
	else
		m_fireLast = 0;
}

void CFuncTank::AdjustAnglesForBarrel(Vector &angles, float distance)
{
	float r2, d2;

	if (m_barrelPos.y != 0 || m_barrelPos.z != 0)
	{
		distance -= m_barrelPos.z;
		d2 = distance * distance;

		if (m_barrelPos.y)
		{
			r2 = m_barrelPos.y * m_barrelPos.y;
			angles.y += (180.0 / M_PI) * atan2(m_barrelPos.y, sqrt(d2 - r2));
		}

		if (m_barrelPos.z)
		{
			r2 = m_barrelPos.z * m_barrelPos.z;
			angles.x += (180.0 / M_PI) * atan2(-m_barrelPos.z, sqrt(d2 - r2));
		}
	}
}
//sohl start
void CFuncTank::TryFire( const Vector &barrelEnd, const Vector &forward, entvars_t *pevAttacker )
{
	if (UTIL_IsMasterTriggered(m_iszFireMaster, NULL))
	{
		Fire( barrelEnd, forward, pevAttacker );
	}
}
//sohl end
void CFuncTank::Fire(const Vector &barrelEnd, const Vector &forward, entvars_t *pevAttacker)
{
	if (m_fireLast != 0)
	{
		if (m_iszSpriteSmoke)
		{
			CSprite *pSprite = CSprite::SpriteCreate(STRING(m_iszSpriteSmoke), barrelEnd, TRUE);
			pSprite->AnimateAndDie(RANDOM_FLOAT(15, 20));
			pSprite->SetTransparency(kRenderTransAlpha, (int)pev->rendercolor.x, (int)pev->rendercolor.y, (int)pev->rendercolor.z, 255, kRenderFxNone);
			pSprite->pev->velocity.z = RANDOM_FLOAT(40, 80);
			pSprite->SetScale(m_spriteScale);
		}

		if (m_iszSpriteFlash)
		{
			CSprite *pSprite = CSprite::SpriteCreate(STRING(m_iszSpriteFlash), barrelEnd, TRUE);
			pSprite->AnimateAndDie(60);
			pSprite->SetTransparency(kRenderTransAdd, 255, 255, 255, 255, kRenderFxNoDissipation);
			pSprite->SetScale(m_spriteScale);
			pSprite->pev->nextthink += 0.1;
		}

		SUB_UseTargets(this, USE_TOGGLE, 0);
	}

	m_fireLast = gpGlobals->time;
}

void CFuncTank::TankTrace(const Vector &vecStart, const Vector &vecForward, const float flSpread, TraceResult &tr)
{
	float x, y, z;

	do
	{
		x = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
		y = RANDOM_FLOAT(-0.5, 0.5) + RANDOM_FLOAT(-0.5, 0.5);
		z = x * x + y * y;
	}
	while (z > 1);

	Vector vecDir = vecForward + x * flSpread * gpGlobals->v_right + y * flSpread * gpGlobals->v_up;
	Vector vecEnd = vecStart + vecDir * 4096;
	UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, edict(), &tr);
}

void CFuncTank::StartRotSound(void)
{
	if (!pev->noise || (pev->spawnflags & SF_TANK_SOUNDON))
		return;

	pev->spawnflags |= SF_TANK_SOUNDON;
	EMIT_SOUND(edict(), CHAN_STATIC, (char *)STRING(pev->noise), 0.85, ATTN_NORM);
}

void CFuncTank::StopRotSound(void)
{
	if (pev->spawnflags & SF_TANK_SOUNDON)
		STOP_SOUND(edict(), CHAN_STATIC, (char *)STRING(pev->noise));

	pev->spawnflags &= ~SF_TANK_SOUNDON;
}

class CFuncTankGun : public CFuncTank
{
public:
	void Fire(const Vector &barrelEnd, const Vector &forward, entvars_t *pevAttacker);
};

LINK_ENTITY_TO_CLASS(func_tank, CFuncTankGun);

void CFuncTankGun::Fire(const Vector &barrelEnd, const Vector &forward, entvars_t *pevAttacker)
{
	if (m_fireLast != 0)
	{
		UTIL_MakeAimVectors(pev->angles);
		int bulletCount = (int)((gpGlobals->time - m_fireLast) * m_fireRate);

		if (bulletCount > 0)
		{
			for (int i = 0; i < bulletCount; i++)
			{
				switch (m_bulletType)
				{
					case TANK_BULLET_9MM: CKFFireBullets(barrelEnd, forward, m_flTankSpread, 4096, BULLET_PLAYER_TF2, m_iBulletDamage, 0, pevAttacker, rand(), TRUE); break;
					case TANK_BULLET_MP5: CKFFireBullets(barrelEnd, forward, m_flTankSpread, 4096, BULLET_PLAYER_TF2, m_iBulletDamage, 0, pevAttacker, rand(), TRUE); break;
					case TANK_BULLET_12MM: CKFFireBullets(barrelEnd, forward, m_flTankSpread, 4096, BULLET_PLAYER_TF2, m_iBulletDamage, 0, pevAttacker, rand(), TRUE); break;

					default:
					case TANK_BULLET_NONE: break;
				}
			}

			CFuncTank::Fire(barrelEnd, forward, pevAttacker);
		}
	}
	else
		CFuncTank::Fire(barrelEnd, forward, pevAttacker);
}

class CFuncTankLaser : public CFuncTank
{
public:
	void Activate(void);
	void KeyValue(KeyValueData *pkvd);
	void Fire(const Vector &barrelEnd, const Vector &forward, entvars_t *pevAttacker);
	void Think(void);
	CLaser *GetLaser(void);

private:
	CLaser *m_pLaser;
	float m_laserTime;
};

LINK_ENTITY_TO_CLASS(func_tanklaser, CFuncTankLaser);

void CFuncTankLaser::Activate(void)
{
	if (!GetLaser())
	{
		UTIL_Remove(this);
		ALERT(at_error, "Laser tank with no env_laser!\n");
	}
	else
		m_pLaser->TurnOff();
}

void CFuncTankLaser::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "laserentity"))
	{
		pev->message = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CFuncTank::KeyValue(pkvd);
}

CLaser *CFuncTankLaser::GetLaser(void)
{
	if (m_pLaser)
		return m_pLaser;

	edict_t *pentLaser = FIND_ENTITY_BY_TARGETNAME(NULL, STRING(pev->message));

	while (!FNullEnt(pentLaser))
	{
		if (FClassnameIs(pentLaser, "env_laser"))
		{
			m_pLaser = (CLaser *)CBaseEntity::Instance(pentLaser);
			break;
		}
		else
			pentLaser = FIND_ENTITY_BY_TARGETNAME(pentLaser, STRING(pev->message));
	}

	return m_pLaser;
}

void CFuncTankLaser::Think(void)
{
	if (m_pLaser && (gpGlobals->time > m_laserTime))
		m_pLaser->TurnOff();

	CFuncTank::Think();
}

void CFuncTankLaser::Fire(const Vector &barrelEnd, const Vector &forward, entvars_t *pevAttacker)
{
	if (m_fireLast != 0 && GetLaser())
	{
		UTIL_MakeAimVectors(pev->angles);
		int bulletCount = (int)((gpGlobals->time - m_fireLast) * m_fireRate);

		if (bulletCount)
		{
			for (int i = 0; i < bulletCount; i++)
			{
				m_pLaser->pev->origin = barrelEnd;

				TraceResult tr;
				TankTrace(barrelEnd, forward, m_flTankSpread, tr);

				m_laserTime = gpGlobals->time;
				m_pLaser->TurnOn();
				m_pLaser->pev->dmgtime = gpGlobals->time - 1;
				m_pLaser->FireAtPoint(tr);
				m_pLaser->pev->nextthink = 0;
			}

			CFuncTank::TryFire(barrelEnd, forward, pev);
		}
	}
	else
		CFuncTank::TryFire(barrelEnd, forward, pev);
}

class CFuncTankRocket : public CFuncTank
{
public:
	void Precache(void);
	void Fire(const Vector &barrelEnd, const Vector &forward, entvars_t *pevAttacker);
};

LINK_ENTITY_TO_CLASS(func_tankrocket, CFuncTankRocket);

void CFuncTankRocket::Precache(void)
{
	UTIL_PrecacheOther("rpg_rocket");
	CFuncTank::Precache();
}

void CFuncTankRocket::Fire(const Vector &barrelEnd, const Vector &forward, entvars_t *pevAttacker)
{
	if (m_fireLast != 0)
	{
		int bulletCount = (int)((gpGlobals->time - m_fireLast) * m_fireRate);

		if (bulletCount > 0)
		{
			for (int i = 0; i < bulletCount; i++)
				CBaseEntity::Create("rpg_rocket", barrelEnd, pev->angles, edict());

			CFuncTank::TryFire(barrelEnd, forward, pev);
		}
	}
	else
		CFuncTank::TryFire(barrelEnd, forward, pev);
}

class CFuncTankMortar : public CFuncTank
{
public:
	void KeyValue(KeyValueData *pkvd);
	void Fire(const Vector &barrelEnd, const Vector &forward, entvars_t *pevAttacker);
};

LINK_ENTITY_TO_CLASS(func_tankmortar, CFuncTankMortar);

void CFuncTankMortar::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "iMagnitude"))
	{
		pev->impulse = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else
		CFuncTank::KeyValue(pkvd);
}

void CFuncTankMortar::Fire(const Vector &barrelEnd, const Vector &forward, entvars_t *pevAttacker)
{
	if (m_fireLast != 0)
	{
		int bulletCount = (int)((gpGlobals->time - m_fireLast) * m_fireRate);

		if (bulletCount > 0)
		{
			UTIL_MakeAimVectors(pev->angles);

			TraceResult tr;
			TankTrace(barrelEnd, forward, m_flTankSpread, tr);
			ExplosionCreate(tr.vecEndPos, pev->angles, edict(), pev->impulse, TRUE);
			CFuncTank::TryFire(barrelEnd, forward, pev);
		}
	}
	else
		CFuncTank::TryFire(barrelEnd, forward, pev);
}

class CFuncTankControls : public CBaseEntity
{
public:
	int ObjectCaps(void);
	void Spawn(void);
	void Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	void Think(void);

public:
	CFuncTank *m_pTank;
};

LINK_ENTITY_TO_CLASS(func_tankcontrols, CFuncTankControls);

int CFuncTankControls::ObjectCaps(void)
{
	return (CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION) | FCAP_IMPULSE_USE;
}

void CFuncTankControls::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (m_pTank)
		m_pTank->Use(pActivator, pCaller, useType, value);
}

void CFuncTankControls::Think(void)
{
	edict_t *pTarget = NULL;

	do pTarget = FIND_ENTITY_BY_TARGETNAME(pTarget, STRING(pev->target));
	while (!FNullEnt(pTarget) && strncmp(STRING(pTarget->v.classname), "func_tank", 9));

	if (FNullEnt(pTarget))
	{
		ALERT(at_console, "No tank %s\n", STRING(pev->target));
		return;
	}

	m_pTank = (CFuncTank*)Instance(pTarget);
}

void CFuncTankControls::Spawn(void)
{
	pev->solid = SOLID_TRIGGER;
	pev->movetype = MOVETYPE_NONE;
	pev->effects |= EF_NODRAW;

	SET_MODEL(ENT(pev), STRING(pev->model));
	UTIL_SetSize(pev, pev->mins, pev->maxs);
	UTIL_SetOrigin(pev, pev->origin);

	pev->nextthink = gpGlobals->time + 0.3;
	CBaseEntity::Spawn();
}