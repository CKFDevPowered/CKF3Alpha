#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

#define CHAN_WEAPON	1

#define STATE_FLAMETHROWER_IDLE 0
#define STATE_FLAMETHROWER_START 1
#define STATE_FLAMETHROWER_LOOP 2
#define STATE_FLAMETHROWER_LOOP_CRIT 3
#define STATE_FLAMETHROWER_AIRBLAST 4

enum flamethrower_e
{
	FLAMETHROWER_IDLE,
	FLAMETHROWER_FIRE,
	FLAMETHROWER_AIRBLAST,
	FLAMETHROWER_DRAW
};

LINK_ENTITY_TO_CLASS(weapon_flamethrower, CFlamethrower);

void CFlamethrower::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_flamethrower");

	Precache();
	m_iId = WEAPON_FLAMETHROWER;
	SET_MODEL(ENT(pev), "models/CKF_III/wp_group_rf.mdl");
	pev->sequence = 23;

	m_iDefaultAmmo = FLAME_DEFAULT_GIVE;
	m_iAmmoConsumption = 0;
	FallInit();
}

void CFlamethrower::Precache(void)
{
	PRECACHE_MODEL("models/CKF_III/v_flamethrower.mdl");
	PRECACHE_SOUND("CKF_III/flamethrower_loop.wav");
	PRECACHE_SOUND("CKF_III/flamethrower_loop_crit.wav");
	PRECACHE_SOUND("CKF_III/flamethrower_start.wav");
	PRECACHE_SOUND("CKF_III/flamethrower_airblast.wav");
	PRECACHE_SOUND("CKF_III/flamethrower_redirect.wav");

	m_usFireScript = PRECACHE_EVENT(1, "events/ckf_flamethrower.sc");
}

int CFlamethrower::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "PrimaryAmmo";
	p->iMaxAmmo1 = _FLAME_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 0;
	p->iPosition = 10;
	p->iId = m_iId = WEAPON_FLAMETHROWER;
	p->iFlags = 0;
	p->iWeight = FLAMETHROWER_WEIGHT;

	return 1;
}

BOOL CFlamethrower::Deploy(void)
{
	m_iAmmoConsumption = 0;
	m_iState = STATE_FLAMETHROWER_IDLE;

	return GroupDeploy("models/CKF_III/v_flamethrower.mdl", "models/CKF_III/wp_group_rf.mdl", FLAMETHROWER_DRAW, 0, 0, "shotgun", 66);
}

void CFlamethrower::Holster(int skiplocal)
{
	m_iState = STATE_FLAMETHROWER_IDLE;

	PLAYBACK_EVENT_FULL(FEV_GLOBAL | FEV_NOTHOST, m_pPlayer->edict(), m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, STATE_FLAMETHROWER_IDLE, 0, 0, 0);
}

void CFlamethrower::PrimaryAttack(void)
{
	FlamethrowerFire();
}

void CFlamethrower::SecondaryAttack(void)
{
	FlamethrowerAirblast();
}

void CFlamethrower::FlamethrowerFire(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;
		if(m_iState != STATE_FLAMETHROWER_IDLE)
		{
			m_iState = STATE_FLAMETHROWER_IDLE;
			PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, m_iState, 0, 0, 0);
			SendWeaponAnim(FLAMETHROWER_IDLE, UseDecrement() != FALSE);
		}
		return;
	}

	UTIL_MakeVectors(m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle);

	Vector vecSrc = m_pPlayer->GetGunPosition();

	TraceResult tr;
	UTIL_TraceLine(vecSrc, vecSrc + gpGlobals->v_forward * 52, ignore_monsters, m_pPlayer->edict(), &tr);

	if( tr.flFraction != 1 )
	{
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;
		if(m_iState != STATE_FLAMETHROWER_IDLE)
		{
			m_iState = STATE_FLAMETHROWER_IDLE;
			PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, m_iState, 0, 0, 0);
			SendWeaponAnim(FLAMETHROWER_IDLE, UseDecrement() != FALSE);
		}
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.1;
		return;
	}

	int iCrit = m_pPlayer->GetCriticalFire(1, m_pPlayer->random_seed);

	m_iAmmoConsumption = (m_iAmmoConsumption + 1) % 2;

	if(m_iAmmoConsumption == 1)
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] --;

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	CFlame *pFlame = CFlame::CreateFlame(vecSrc, m_pPlayer->pev->v_angle, m_pPlayer);

	pFlame->m_iCrit = iCrit;
	pFlame->m_iTeam = m_pPlayer->m_iTeam;

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.04;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.15;

	if(m_iState == STATE_FLAMETHROWER_IDLE)
	{
		m_iState = STATE_FLAMETHROWER_START;
		m_flState = UTIL_WeaponTimeBase() + 1.0;
		PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, m_iState, 0, 0, 0);
		SendWeaponAnim(FLAMETHROWER_FIRE, UseDecrement() != FALSE);
	}
	else if(m_iState >= STATE_FLAMETHROWER_START && UTIL_WeaponTimeBase() > m_flState)
	{
		m_iState = (iCrit >= 2) ? STATE_FLAMETHROWER_LOOP_CRIT : STATE_FLAMETHROWER_LOOP;
		m_flState = UTIL_WeaponTimeBase() + 3.5;
		PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, m_iState, m_pPlayer->m_iTeam, 0, 0);
		SendWeaponAnim(FLAMETHROWER_FIRE, UseDecrement() != FALSE);
	}
	else if(iCrit >= 2 && m_iState == STATE_FLAMETHROWER_LOOP)
	{
		m_iState = STATE_FLAMETHROWER_LOOP_CRIT;
		m_flState = UTIL_WeaponTimeBase() + 3.5;
		PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, m_iState, m_pPlayer->m_iTeam, 0, 0);
		SendWeaponAnim(FLAMETHROWER_FIRE, UseDecrement() != FALSE);
	}
	else if(iCrit < 2 && m_iState == STATE_FLAMETHROWER_LOOP_CRIT)
	{
		m_iState = STATE_FLAMETHROWER_LOOP;
		m_flState = UTIL_WeaponTimeBase() + 3.5;
		PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, m_iState, 0, 0, 0);
		SendWeaponAnim(FLAMETHROWER_FIRE, UseDecrement() != FALSE);
	}
}

void CFlamethrower::FlamethrowerAirblast(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] < 20)
	{
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2;
		m_iState = STATE_FLAMETHROWER_IDLE;
		return;
	}

	UTIL_MakeVectors(m_pPlayer->pev->v_angle);

	Vector vecSrc = m_pPlayer->GetGunPosition() + gpGlobals->v_forward * 16;

	CKFAirblast(vecSrc, gpGlobals->v_forward, pev, m_pPlayer->pev, 0, 300, 150);

	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] -= 20;

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.75;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.75;

	if (m_iClip)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5;
	else
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.80;

	m_iState = STATE_FLAMETHROWER_IDLE;
	PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, STATE_FLAMETHROWER_AIRBLAST, 0, 0, 0);
	SendWeaponAnim(FLAMETHROWER_AIRBLAST, UseDecrement() != FALSE);
}

void CFlamethrower::WeaponIdle(void)
{
	ResetEmptySound();

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if(m_iState != STATE_FLAMETHROWER_IDLE)
	{
		m_iState = STATE_FLAMETHROWER_IDLE;
		PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, STATE_FLAMETHROWER_IDLE, 0, 0, 0);
		SendWeaponAnim(FLAMETHROWER_IDLE, UseDecrement() != FALSE);
	}
}

void CFlamethrower::ItemPostFrame(void)
{
	CBasePlayerWeapon::ItemPostFrame();
}

LINK_ENTITY_TO_CLASS(pj_flame, CFlame);

CFlame *CFlame::CreateFlame(Vector vecOrigin, Vector vecAngles, CBaseEntity *pOwner)
{
	CFlame *pFlame = GetClassPtr( (CFlame *)NULL );

	UTIL_SetOrigin( pFlame->pev, vecOrigin );
	vecAngles.x *= -1;
	pFlame->pev->angles = vecAngles;

	pFlame->Spawn();
	pFlame->SetTouch(&CFlame::FlameTouch);
	pFlame->pev->owner = pOwner->edict();

	pFlame->pev->velocity = gpGlobals->v_forward * 400 + pOwner->pev->velocity;

	pFlame->SetThink(&CFlame::FlameThink);
	pFlame->pev->nextthink = gpGlobals->time + 0.01;
	pFlame->m_fFireTime = gpGlobals->time;
	pFlame->m_fDmgTime = gpGlobals->time;

	return pFlame;
}

void CFlame::Spawn( void )
{
	Precache();

	UTIL_SetSize(pev, Vector(-1,-1,-1), Vector(1,1,1));
	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_TRIGGER;
	pev->takedamage = DAMAGE_NO;
	pev->effects |= EF_NODRAW;

	UTIL_SetOrigin( pev, pev->origin );
	pev->gravity = 0.00001;
	pev->health = 99999;

	pev->classname = MAKE_STRING("pj_flame");

	m_fDmg = 6.2;
	m_fDmgRadius = 64;
	m_iPjFlags = 0;
	m_pDeflecter = NULL;
}

void CFlame::FlameThink(void)
{
	if(gpGlobals->time - m_fFireTime > 0.6f)
	{
		UTIL_Remove(this);
		return;
	}
	if(m_fDmgTime < gpGlobals->time)
	{
		entvars_t *pevOwner = NULL;
		if (pev->owner)
			pevOwner = VARS(pev->owner);

		CKFRadiusDamage(pev->origin, NULL, pev, pevOwner, max(m_fDmg-0.4*(gpGlobals->time-m_fFireTime)/0.4,m_fDmg*0.6) , m_fDmgRadius, 0, DMG_NEVERGIB|DMG_NOSELFDMG|DMG_FLAME, 0, 0, m_iCrit);
		m_fDmgTime = gpGlobals->time + 0.1;
	}
	if( pev->velocity.Length() > 100 )
	{
		float flAcclerate = pev->velocity.Length()/20;
		pev->velocity = pev->velocity - pev->velocity.Normalize() * flAcclerate;
	}
	pev->nextthink = gpGlobals->time + 0.01;
}

void CFlame::FlameTouch(CBaseEntity *pOther)
{
	if(pOther->IsBSPModel())
	{
		UTIL_Remove(this);
	}
}

void CFlame::Precache( void )
{
}