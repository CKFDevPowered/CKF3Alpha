#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "trains.h"
#include "vehicle.h"

LINK_ENTITY_TO_CLASS(func_vehicle, CFuncVehicle);

void CFuncVehicle::KeyValue(KeyValueData *pkvd)
{
	if (FStrEq(pkvd->szKeyName, "length"))
	{
		m_length = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "width"))
	{
		m_width = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "height"))
	{
		m_height = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "startspeed"))
	{
		m_startSpeed = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "sounds"))
	{
		m_sounds = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "volume"))
	{
		m_flVolume = (float)atoi(pkvd->szValue);
		m_flVolume *= 0.1;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "bank"))
	{
		m_flBank = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "acceleration"))
	{
		m_acceleration = atoi(pkvd->szValue);

		if (m_acceleration < 1)
			m_acceleration = 1;
		else if (m_acceleration > 10)
			m_acceleration = 10;

		pkvd->fHandled = TRUE;
	}
	else
		CBaseEntity::KeyValue(pkvd);
}

void CFuncVehicle::NextThink(float thinkTime, BOOL alwaysThink)
{
	if (alwaysThink)
		pev->flags |= FL_ALWAYSTHINK;
	else
		pev->flags &= ~FL_ALWAYSTHINK;

	pev->nextthink = thinkTime;
}

void CFuncVehicle::Blocked(CBaseEntity *pOther)
{
	entvars_t *pevOther = pOther->pev;

	if (FBitSet(pevOther->flags, FL_ONGROUND) && VARS(pevOther->groundentity) == pev)
	{
		pevOther->velocity = pev->velocity;
		return;
	}
	else
	{
		pevOther->velocity = (pevOther->origin - pev->origin).Normalize() * pev->dmg;
		pevOther->velocity.z += 300;
		pev->velocity = pev->velocity * 0.85;
	}

	ALERT(at_aiconsole, "TRAIN(%s): Blocked by %s (dmg:%.2f)\n", STRING(pev->targetname), STRING(pOther->pev->classname), pev->dmg);
	UTIL_MakeVectors(pev->angles);

	Vector vecLength = (gpGlobals->v_forward * -1) * (m_length * 0.5);
	Vector vecWidth = (gpGlobals->v_right * -1) * (m_width * 0.5);
	Vector vecAbsMin = pev->origin + vecLength - vecWidth;
	Vector vecAbsMax = pev->origin - vecLength + vecWidth;
	float left = min(vecAbsMin.x, vecAbsMax.x);
	float right = max(vecAbsMin.x, vecAbsMax.x);
	float top = min(vecAbsMin.y, vecAbsMax.y);
	float bottom = max(vecAbsMin.y, vecAbsMax.y);
	float height = pev->origin.z + (2 * abs((int)(pev->mins.z - pev->maxs.z)));

	if (pOther->pev->origin.x < left || pOther->pev->origin.x > right || pOther->pev->origin.y < top || pOther->pev->origin.y > bottom || pOther->pev->origin.z < pev->origin.z || pOther->pev->origin.z > height)
		pOther->TakeDamage(pev, pev, 150, DMG_CRUSH, 0);
}

void CFuncVehicle::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	if (useType != USE_SET)
	{
		if (!ShouldToggle(useType, (pev->speed != 0)))
			return;

		if (pev->speed == 0)
		{
			pev->speed = m_speed * m_dir;
			Next();
		}
		else
		{
			pev->speed = 0;
			pev->velocity = g_vecZero;
			pev->avelocity = g_vecZero;
			StopSound();
			SetThink(NULL);
		}
	}

	if (value < 10)
	{
		if (value < 0 && pev->speed > 145)
			StopSound();

		float delta = value;

		if (value > 0)
		{
			delta = pev->speed / m_speed;

			if (pev->speed < 0)
				delta = m_acceleration * 0.0005 + delta + VEHICLE_SPEED0_ACCELERATION;
			else if (pev->speed < 10)
				delta = m_acceleration * 0.0006 + delta + VEHICLE_SPEED1_ACCELERATION;
			else if (pev->speed < 20)
				delta = m_acceleration * 0.0007 + delta + VEHICLE_SPEED2_ACCELERATION;
			else if (pev->speed < 30)
				delta = m_acceleration * 0.0007 + delta + VEHICLE_SPEED3_ACCELERATION;
			else if (pev->speed < 45)
				delta = m_acceleration * 0.0007 + delta + VEHICLE_SPEED4_ACCELERATION;
			else if (pev->speed < 60)
				delta = m_acceleration * 0.0008 + delta + VEHICLE_SPEED5_ACCELERATION;
			else if (pev->speed < 80)
				delta = m_acceleration * 0.0008 + delta + VEHICLE_SPEED6_ACCELERATION;
			else if (pev->speed < 100)
				delta = m_acceleration * 0.0009 + delta + VEHICLE_SPEED7_ACCELERATION;
			else if (pev->speed < 150)
				delta = m_acceleration * 0.0008 + delta + VEHICLE_SPEED8_ACCELERATION;
			else if (pev->speed < 225)
				delta = m_acceleration * 0.0007 + delta + VEHICLE_SPEED9_ACCELERATION;
			else if (pev->speed < 300)
				delta = m_acceleration * 0.0006 + delta + VEHICLE_SPEED10_ACCELERATION;
			else if (pev->speed < 400)
				delta = m_acceleration * 0.0005 + delta + VEHICLE_SPEED11_ACCELERATION;
			else if (pev->speed < 550)
				delta = m_acceleration * 0.0005 + delta + VEHICLE_SPEED12_ACCELERATION;
			else if (pev->speed < 800)
				delta = m_acceleration * 0.0005 + delta + VEHICLE_SPEED13_ACCELERATION;
			else
				delta = m_acceleration * 0.0005 + delta + VEHICLE_SPEED14_ACCELERATION;
		}
		else if (value < 0)
		{
			delta = pev->speed / m_speed;

			if (delta > 0)
				delta -= 0.0125;
			else if (delta <= 0 && delta > -0.05)
				delta -= 0.0075;
			else if (delta <= 0.05 && delta > -0.1)
				delta -= 0.01;
			else if (delta <= 0.15 && delta > -0.15)
				delta -= 0.0125;
			else if (delta <= 0.15 && delta > -0.22)
				delta -= 0.01375;
			else if (delta <= 0.22 && delta > -0.3)
				delta -= - 0.0175;
			else if (delta <= 0.3)
				delta -= 0.0125;
		}

		if (delta > 1)
			delta = 1;
		else if (delta < -0.35)
			delta = -0.35;

		pev->speed = m_speed * delta;
		Next();
		m_flCheckSpeedTime = gpGlobals->time + 0.25;
	}
	else
	{
		if (gpGlobals->time > m_flNextTurningTime)
		{
			if (value == 20)
			{
				m_iTurning++;
				m_flCheckTurningTime = gpGlobals->time + 0.075;

				if (m_iTurning > 8)
					m_iTurning = 8;
			}
			else if (value == 30)
			{
				m_iTurning--;
				m_flCheckTurningTime = gpGlobals->time + 0.075;

				if (m_iTurning < -8)
					m_iTurning = -8;
			}

			m_flNextTurningTime = gpGlobals->time + 0.05;
		}
	}
}

static float Fix(float angle)
{
	while (angle < 0)
		angle += 360;
	while (angle > 360)
		angle -= 360;

	return angle;
}

static void FixupAngles(Vector &v)
{
	v.x = Fix(v.x);
	v.y = Fix(v.y);
	v.z = Fix(v.z);
}

#define VEHICLE_STARTPITCH 60
#define VEHICLE_MAXPITCH 200
#define VEHICLE_MAXSPEED 1500

void CFuncVehicle::StopSound(void)
{
	if (m_soundPlaying && pev->noise)
	{
		unsigned short us_sound = ((unsigned short)m_sounds & 0x0007) << 12;
		unsigned short us_encode = us_sound;

		PLAYBACK_EVENT_FULL(FEV_RELIABLE | FEV_UPDATE, edict(), m_usAdjustPitch, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, us_encode, 0, 1, 0);
	}

	m_soundPlaying = 0;
}

void CFuncVehicle::UpdateSound(void)
{
	if (!pev->noise)
		return;

	float flpitch = VEHICLE_STARTPITCH + (abs((int)pev->speed) * (VEHICLE_MAXPITCH - VEHICLE_STARTPITCH) / VEHICLE_MAXSPEED);

	if (flpitch > 200)
		flpitch = 200;

	if (!m_soundPlaying)
	{
		if (m_sounds < 5)
			EMIT_SOUND_DYN(ENT(pev), CHAN_ITEM, "plats/vehicle_brake1.wav", m_flVolume, ATTN_NORM, 0, 100);

		EMIT_SOUND_DYN(ENT(pev), CHAN_STATIC, (char *)STRING(pev->noise), m_flVolume, ATTN_NORM, 0, (int)flpitch);
		m_soundPlaying = 1;
	}
	else
	{
		unsigned short us_sound = ((unsigned short)(m_sounds) & 0x0007) << 12;
		unsigned short us_pitch = ((unsigned short)(flpitch / 10.0) & 0x003F) << 6;
		unsigned short us_volume = ((unsigned short)(m_flVolume * 40) & 0x003F);
		unsigned short us_encode = us_sound | us_pitch | us_volume;

		PLAYBACK_EVENT_FULL(FEV_RELIABLE | FEV_UPDATE, edict(), m_usAdjustPitch, 0.0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0, 0.0, us_encode, 0, 0, 0);
	}
}

void CFuncVehicle::CheckTurning(void)
{
	TraceResult tr;
	Vector vecStart, vecEnd;

	if (m_iTurning < 0)
	{
		if (pev->speed > 0)
		{
			vecStart = m_vecLeftBottom;
			vecEnd = vecStart - gpGlobals->v_right * 16;
		}
		else if (pev->speed < 0)
		{
			vecStart = m_vecRightTop;
			vecEnd = vecStart + gpGlobals->v_right * 16;
		}

		UTIL_TraceLine(vecStart, vecEnd, ignore_monsters, dont_ignore_glass, ENT(pev), &tr);

		if (tr.flFraction != 1)
			m_iTurning = 1;
	}
	else if (m_iTurning > 0)
	{
		if (pev->speed > 0)
		{
			vecStart = m_vecRightBottom;
			vecEnd = vecStart + gpGlobals->v_right * 16;
		}
		else if (pev->speed < 0)
		{
			vecStart = m_vecLeftTop;
			vecEnd = vecStart - gpGlobals->v_right * 16;
		}

		UTIL_TraceLine(vecStart, vecEnd, ignore_monsters, dont_ignore_glass, ENT(pev), &tr);

		if (tr.flFraction != 1)
			m_iTurning = -1;
	}

	if (pev->speed <= 0)
		return;

	float speed;
	int turning = abs(m_iTurning);

	if (turning > 4)
	{
		if (m_deceleration != -1)
		{
			float time = gpGlobals->time - m_deceleration;

			if (time >= 0)
				speed = m_speed * 0.98;
			else if (time > 0.3)
				speed = m_speed * 0.95;
			else if (time > 0.6)
				speed = m_speed * 0.9;
			else if (time > 0.8)
				speed = m_speed * 0.8;
			else if (time > 1)
				speed = m_speed * 0.7;
			else if (time > 1.2)
				speed = m_speed * 0.5;
			else
				speed = time;
		}
		else
		{
			m_deceleration = gpGlobals->time;
			speed = m_speed;
		}
	}
	else
	{
		m_deceleration = -1;

		if (turning > 2)
			speed = m_speed * 0.9;
		else
			speed = m_speed;
	}

	if (speed < pev->speed)
		pev->speed -= m_speed * 0.1;
}

void CFuncVehicle::CollisionDetection(void)
{
	TraceResult tr;
	Vector vecStart, vecEnd;

	if (pev->speed < 0)
	{
		vecStart = m_vecRightTop;
		vecEnd = vecStart + (gpGlobals->v_forward * 16);
		UTIL_TraceLine(vecStart, vecEnd, ignore_monsters, dont_ignore_glass, ENT(pev), &tr);

		if (tr.flFraction != 1)
		{
			float flDot = DotProduct(gpGlobals->v_forward, tr.vecPlaneNormal * -1);

			if (flDot < 0.7 && tr.vecPlaneNormal.z < 0.1)
			{
				m_planeNormal = tr.vecPlaneNormal;
				m_planeNormal.z = 0;
				pev->speed *= 0.99;
			}
			else if (tr.vecPlaneNormal.z < 0.65 || tr.fStartSolid)
				pev->speed *= -1;
			else
				m_planeNormal = tr.vecPlaneNormal;

			CBaseEntity *pHit = CBaseEntity::Instance(tr.pHit);

			if (pHit && pHit->Classify() == CLASS_VEHICLE)
				ALERT(at_console, "I hit another vehicle\n");
		}

		vecStart = m_vecLeftTop;
		vecEnd = vecStart + (gpGlobals->v_forward * 16);
		UTIL_TraceLine(vecStart, vecEnd, ignore_monsters, dont_ignore_glass, ENT(pev), &tr);

		if (tr.flFraction == 1)
		{
			vecStart = m_vecTop;
			vecEnd = vecStart + (gpGlobals->v_forward * 16);
			UTIL_TraceLine(vecStart, vecEnd, ignore_monsters, dont_ignore_glass, ENT(pev), &tr);

			if (tr.flFraction == 1)
				return;
		}

		float flDot = DotProduct(gpGlobals->v_forward, tr.vecPlaneNormal * -1);

		if (flDot >= 0.7)
		{
			if (tr.vecPlaneNormal.z < 0.65 || tr.fStartSolid != FALSE)
				pev->speed *= -1;
			else
				m_planeNormal = tr.vecPlaneNormal;
		}
		else if (tr.vecPlaneNormal.z < 0.1)
		{
			m_planeNormal = tr.vecPlaneNormal;
			m_planeNormal.z = 0;
			pev->speed *= 0.99;
		}
		else if (tr.vecPlaneNormal.z < 0.65 || tr.fStartSolid != FALSE)
			pev->speed *= -1;
		else
			m_planeNormal = tr.vecPlaneNormal;
	}
	else if (pev->speed > 0)
	{
		vecStart = m_vecRightBottom;
		vecEnd = vecStart - (gpGlobals->v_forward * 16);
		UTIL_TraceLine(vecStart, vecEnd, dont_ignore_monsters, dont_ignore_glass, ENT(pev), &tr);

		if (tr.flFraction == 1)
		{
			vecStart = m_vecLeftBottom;
			vecEnd = vecStart - (gpGlobals->v_forward * 16);
			UTIL_TraceLine(vecStart, vecEnd, ignore_monsters, dont_ignore_glass, ENT(pev), &tr);

			if (tr.flFraction == 1)
			{
				vecStart = m_vecBottom;
				vecEnd = vecStart - (gpGlobals->v_forward * 16);
				UTIL_TraceLine(vecStart, vecEnd, ignore_monsters, dont_ignore_glass, ENT(pev), &tr);

				if (tr.flFraction == 1)
					return;
			}
		}

		float flDot = DotProduct(gpGlobals->v_forward, tr.vecPlaneNormal * -1);

		if (flDot <= -0.7)
		{
			if (tr.vecPlaneNormal.z < 0.65 || tr.fStartSolid != FALSE)
				pev->speed *= -1;
			else
				m_planeNormal = tr.vecPlaneNormal;
		}
		else if (tr.vecPlaneNormal.z < 0.1)
		{
			m_planeNormal = tr.vecPlaneNormal;
			m_planeNormal.z = 0;
			pev->speed *= 0.99;
		}
		else if (tr.vecPlaneNormal.z < 0.65 || tr.fStartSolid != FALSE)
			pev->speed *= -1;
		else
			m_planeNormal = tr.vecPlaneNormal;
	}
}

void CFuncVehicle::TerrainFollowing(void)
{
	TraceResult tr;
	UTIL_TraceLine(pev->origin, pev->origin + Vector(0, 0, (m_height + 48) * -1), ignore_monsters, dont_ignore_glass, ENT(pev), &tr);

	if (tr.flFraction != 1)
		m_planeNormal = tr.vecPlaneNormal;
	else if (tr.fInWater)
		m_planeNormal = Vector(0, 0, 1);
}

void CFuncVehicle::Next(void)
{
	Vector vecGallop = g_vecZero;
	UTIL_MakeVectors(pev->angles);

	Vector vecLength = (gpGlobals->v_forward * -1) * (m_length * 0.5);
	Vector vecWidth = (gpGlobals->v_right * -1) * (m_width * 0.5);
	Vector vecHeight = gpGlobals->v_up * 16;

	m_vecRightBottom = pev->origin + vecLength - vecWidth + vecHeight;
	m_vecLeftBottom = pev->origin + vecLength + vecWidth + vecHeight;
	m_vecBottom = pev->origin + vecLength + vecHeight;
	m_vecRightTop = pev->origin - vecLength - vecWidth + vecHeight;
	m_vecLeftTop = pev->origin - vecLength + vecWidth + vecHeight;
	m_vecTop = pev->origin - vecLength + vecHeight;
	m_planeNormal = g_vecZero;

	CheckTurning();

	if (gpGlobals->time > m_flCheckTurningTime)
	{
		m_flCheckTurningTime = gpGlobals->time + 0.1;

		if (m_iTurning < 0)
			m_iTurning++;
		else if (m_iTurning > 0)
			m_iTurning--;
	}

	if (gpGlobals->time > m_flCheckSpeedTime)
	{
		if (pev->speed < 0)
		{
			pev->speed += 20;

			if (pev->speed > 0)
				pev->speed = 0;
		}
		else if (pev->speed > 0)
		{
			pev->speed -= 20;

			if (pev->speed < 0)
				pev->speed = 0;
		}
	}

	if (pev->speed == 0)
	{
		m_iTurning = 0;
		pev->avelocity = g_vecZero;
		pev->velocity = g_vecZero;
		SetThink(&CFuncVehicle::Next);
		NextThink(pev->ltime + 0.1, TRUE);
		return;
	}

	TerrainFollowing();
	CollisionDetection();

	if (m_planeNormal == g_vecZero)
	{
		if (m_flStartGallopTime != -1)
		{
			vecGallop = Vector(0, 0, 0);
			vecGallop.z = (gpGlobals->time - m_flStartGallopTime) * -35;

			if (vecGallop.z < -400)
				vecGallop.z = -400;
		}
		else
		{
			m_flStartGallopTime = gpGlobals->time;
			vecGallop = Vector(0, 0, 0);
			pev->velocity = pev->velocity * 1.5;
		}

		m_direction = gpGlobals->v_forward * -1;
	}
	else
	{
		m_direction = CrossProduct(m_planeNormal, gpGlobals->v_forward);
		m_direction = CrossProduct(m_planeNormal, m_direction);

		Vector angles = UTIL_VecToAngles(m_direction);
		angles.y += 180;

		if (m_iTurning != 0)
			angles.y += m_iTurning;

		FixupAngles(angles);
		FixupAngles(pev->angles);

		float vy, vx;
		vx = UTIL_AngleDistance(angles.x, pev->angles.x);
		vy = UTIL_AngleDistance(angles.y, pev->angles.y);

		if (vx > 10)
			vx = 10;
		else if (vx < -10)
			vx = -10;

		if (vy > 10)
			vy = 10;
		else if (vy < -10)
			vy = -10;

		pev->avelocity.y = (int)(vy * 10);
		pev->avelocity.x = (int)(vx * 10);
		m_flStartGallopTime = -1;
	}

	UTIL_VecToAngles(m_direction);

	if (gpGlobals->time > m_flNextUpdateSound)
	{
		UpdateSound();
		m_flNextUpdateSound = gpGlobals->time + 1;
	}

	if (m_planeNormal == g_vecZero)
		pev->velocity = pev->velocity + vecGallop;
	else
		pev->velocity = m_direction.Normalize() * pev->speed;

	SetThink(&CFuncVehicle::Next);
	NextThink(pev->ltime + 0.1, TRUE);
}

void CFuncVehicle::DeadEnd(void)
{
	CPathTrack *pTrack = m_ppath;
	ALERT(at_aiconsole, "TRAIN(%s): Dead end ", STRING(pev->targetname));

	if (pTrack)
	{
		CPathTrack *pNext;

		if (m_oldSpeed < 0)
		{
			do
			{
				pNext = pTrack->ValidPath(pTrack->GetPrevious(), TRUE);

				if (pNext)
					pTrack = pNext;
			}
			while (pNext);
		}
		else
		{
			do
			{
				pNext = pTrack->ValidPath(pTrack->GetNext(), TRUE);

				if (pNext)
					pTrack = pNext;
			}
			while (pNext);
		}
	}

	pev->velocity = g_vecZero;
	pev->avelocity = g_vecZero;

	if (pTrack)
	{
		ALERT(at_aiconsole, "at %s\n", STRING(pTrack->pev->targetname));

		if (pTrack->pev->netname)
			FireTargets(STRING(pTrack->pev->netname), this, this, USE_TOGGLE, 0);
	}
	else
		ALERT(at_aiconsole, "\n");
}

void CFuncVehicle::SetControls(entvars_t *pevControls)
{
	Vector offset = pevControls->origin - pev->oldorigin;
	m_controlMins = pevControls->mins + offset;
	m_controlMaxs = pevControls->maxs + offset;
}

BOOL CFuncVehicle::OnControls(entvars_t *pevTest)
{
	Vector offset = pevTest->origin - pev->origin;

	if (pev->spawnflags & SF_TRACKTRAIN_NOCONTROL)
		return FALSE;

	UTIL_MakeVectors(pev->angles);

	Vector local;
	local.x = DotProduct(offset, gpGlobals->v_forward);
	local.y = -DotProduct(offset, gpGlobals->v_right);
	local.z = DotProduct(offset, gpGlobals->v_up);

	if (local.x >= m_controlMins.x && local.y >= m_controlMins.y && local.z >= m_controlMins.z && local.x <= m_controlMaxs.x && local.y <= m_controlMaxs.y && local.z <= m_controlMaxs.z)
		return TRUE;

	return FALSE;
}

void CFuncVehicle::Find(void)
{
	m_ppath = CPathTrack::Instance(FIND_ENTITY_BY_TARGETNAME(NULL, STRING(pev->target)));

	if (!m_ppath)
		return;

	entvars_t *pevTarget = m_ppath->pev;

	if (!FClassnameIs(pevTarget, "path_track"))
	{
		ALERT(at_error, "func_track_train must be on a path of path_track\n");
		m_ppath = NULL;
		return;
	}

	Vector nextPos = pevTarget->origin;
	nextPos.z += m_height;

	Vector look = nextPos;
	look.z -= m_height;
	m_ppath->LookAhead(&look, m_length, 0);
	look.z += m_height;

	pev->angles = UTIL_VecToAngles(look - nextPos);
	pev->angles.y += 180;

	if (pev->spawnflags & SF_TRACKTRAIN_NOPITCH)
		pev->angles.x = 0;

	UTIL_SetOrigin(pev, nextPos);
	NextThink(pev->ltime + 0.1, FALSE);
	SetThink(&CFuncVehicle::Next);
	pev->speed = m_startSpeed;
	UpdateSound();
}

void CFuncVehicle::NearestPath(void)
{
	CBaseEntity *pTrack = NULL;
	CBaseEntity *pNearest = NULL;
	float closest = 1024;

	while ((pTrack = UTIL_FindEntityInSphere(pTrack, pev->origin, 1024)) != NULL)
	{
		if (!(pTrack->pev->flags & (FL_CLIENT | FL_MONSTER)) && FClassnameIs(pTrack->pev, "path_track"))
		{
			float dist = (pev->origin - pTrack->pev->origin).Length();

			if (dist < closest)
			{
				closest = dist;
				pNearest = pTrack;
			}
		}
	}

	if (!pNearest)
	{
		ALERT(at_console, "Can't find a nearby track !!!\n");
		SetThink(NULL);
		return;
	}

	ALERT(at_aiconsole, "TRAIN: %s, Nearest track is %s\n", STRING(pev->targetname), STRING(pNearest->pev->targetname));
	pTrack = ((CPathTrack *)pNearest)->GetNext();

	if (pTrack)
	{
		if ((pev->origin - pTrack->pev->origin).Length() < (pev->origin - pNearest->pev->origin).Length())
			pNearest = pTrack;
	}

	m_ppath = (CPathTrack *)pNearest;

	if (pev->speed != 0)
	{
		NextThink(pev->ltime + 0.1, FALSE);
		SetThink(&CFuncVehicle::Next);
	}
}

void CFuncVehicle::OverrideReset(void)
{
	NextThink(pev->ltime + 0.1, FALSE);
	SetThink(&CFuncVehicle::NearestPath);
}

CFuncVehicle *CFuncVehicle::Instance(edict_t *pent)
{
	if (FClassnameIs(pent, "func_vehicle"))
		return (CFuncVehicle *)GET_PRIVATE(pent);

	return NULL;
}

int CFuncVehicle::Classify(void)
{
	return CLASS_VEHICLE;
}

void CFuncVehicle::Spawn(void)
{
	if (pev->speed == 0)
		m_speed = 165;
	else
		m_speed = pev->speed;

	if (!m_sounds)
		m_sounds = 3;

	ALERT(at_console, "M_speed = %f\n", m_speed);

	pev->speed = 0;
	pev->velocity = g_vecZero;
	pev->avelocity = g_vecZero;
	pev->impulse = (int)m_speed;
	m_acceleration = 5;
	m_dir = 1;
	m_deceleration = -1;

	if (FStringNull(pev->target))
		ALERT(at_console, "Vehicle with no target");

	if (pev->spawnflags & SF_TRACKTRAIN_PASSABLE)
		pev->solid = SOLID_NOT;
	else
		pev->solid = SOLID_BSP;

	pev->movetype = MOVETYPE_PUSH;

	SET_MODEL(ENT(pev), STRING(pev->model));
	UTIL_SetSize(pev, pev->mins, pev->maxs);
	UTIL_SetOrigin(pev, pev->origin);

	pev->oldorigin = pev->origin;
	m_controlMins = pev->mins;
	m_controlMaxs = pev->maxs;
	m_controlMaxs.z += 72;

	NextThink(pev->ltime + 0.1, FALSE);
	SetThink(&CFuncVehicle::Find);
	Precache();
}

void CFuncVehicle::Restart(void)
{
	ALERT(at_console, "M_speed = %f\n", m_speed);

	pev->speed = 0;
	pev->velocity = g_vecZero;
	pev->avelocity = g_vecZero;
	pev->impulse = (int)m_speed;
	m_deceleration = -1;
	m_flNextUpdateSound = -1;
	m_dir = 1;
	m_pDriver = NULL;

	if (FStringNull(pev->target))
		ALERT(at_console, "Vehicle with no target");

	UTIL_SetOrigin(pev, pev->oldorigin);
	NextThink(pev->ltime + 0.1, FALSE);
	SetThink(&CFuncVehicle::Find);
}

void CFuncVehicle::Precache(void)
{
	if (m_flVolume == 0)
		m_flVolume = 1;

	switch (m_sounds)
	{
		case 1: PRECACHE_SOUND("plats/vehicle1.wav"); pev->noise = MAKE_STRING("plats/vehicle1.wav"); break;
		case 2: PRECACHE_SOUND("plats/vehicle2.wav"); pev->noise = MAKE_STRING("plats/vehicle2.wav"); break;
		case 3: PRECACHE_SOUND("plats/vehicle3.wav"); pev->noise = MAKE_STRING("plats/vehicle3.wav"); break;
		case 4: PRECACHE_SOUND("plats/vehicle4.wav"); pev->noise = MAKE_STRING("plats/vehicle4.wav"); break;
		case 5: PRECACHE_SOUND("plats/vehicle6.wav"); pev->noise = MAKE_STRING("plats/vehicle6.wav"); break;
		case 6: PRECACHE_SOUND("plats/vehicle7.wav"); pev->noise = MAKE_STRING("plats/vehicle7.wav"); break;
	}

	PRECACHE_SOUND("plats/vehicle_brake1.wav");
	PRECACHE_SOUND("plats/vehicle_start1.wav");
	m_usAdjustPitch = PRECACHE_EVENT(1, "events/vehicle.sc");
}

class CFuncVehicleControls : public CBaseEntity
{
public:
	int ObjectCaps(void) { return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }
	void Spawn(void);
	void EXPORT Find(void);
};

LINK_ENTITY_TO_CLASS(func_vehiclecontrols, CFuncVehicleControls);

void CFuncVehicleControls::Find(void)
{
	edict_t *pTarget = NULL;
	do pTarget = FIND_ENTITY_BY_TARGETNAME(pTarget, STRING(pev->target));
	while (!FNullEnt(pTarget) && !FClassnameIs(pTarget, "func_vehicle"));

	if (FNullEnt(pTarget))
	{
		ALERT(at_console, "No vehicle %s\n", STRING(pev->target));
		return;
	}

	CFuncVehicle *ptrain = CFuncVehicle::Instance(pTarget);
	ptrain->SetControls(pev);
	UTIL_Remove(this);
}

void CFuncVehicleControls::Spawn(void)
{
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;
	SET_MODEL(ENT(pev), STRING(pev->model));

	UTIL_SetSize(pev, pev->mins, pev->maxs);
	UTIL_SetOrigin(pev, pev->origin);

	SetThink(&CFuncVehicleControls::Find);
	pev->nextthink = gpGlobals->time;
}