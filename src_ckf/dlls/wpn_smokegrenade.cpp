#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

enum smokegrenade_e
{
	SMOKEGRENADE_IDLE,
	SMOKEGRENADE_PULLPIN,
	SMOKEGRENADE_THROW,
	SMOKEGRENADE_DEPLOY
};

LINK_ENTITY_TO_CLASS(weapon_smokegrenade, CSmokeGrenade);

void CSmokeGrenade::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_smokegrenade");

	Precache();
	m_iId = WEAPON_SMOKEGRENADE;
	SET_MODEL(ENT(pev), "models/w_smokegrenade.mdl");

	pev->dmg = 4;
	m_iDefaultAmmo = SMOKEGRENADE_DEFAULT_GIVE;
	m_flStartThrow = 0;
	m_flReleaseThrow = -1;

	FallInit();
}

void CSmokeGrenade::Precache(void)
{
	PRECACHE_MODEL("models/v_smokegrenade.mdl");
	PRECACHE_SOUND("weapons/pinpull.wav");
	PRECACHE_SOUND("weapons/sg_explode.wav");

	m_usCreateExplo = PRECACHE_EVENT(1, "events/createsmoke.sc");
}

int CSmokeGrenade::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "SmokeGrenade";
	p->iMaxAmmo1 = SMOKEGRENADE_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 3;
	p->iPosition = 3;
	p->iId = m_iId = WEAPON_SMOKEGRENADE;
	p->iWeight = SMOKEGRENADE_WEIGHT;
	p->iFlags = ITEM_FLAG_LIMITINWORLD | ITEM_FLAG_EXHAUSTIBLE;

	return 1;
}

BOOL CSmokeGrenade::Deploy(void)
{
	m_flReleaseThrow = -1;
	return CBasePlayerWeapon::DefaultDeploy("models/v_smokegrenade.mdl", "models/p_smokegrenade.mdl", SMOKEGRENADE_DEPLOY, "grenade", UseDecrement() != FALSE);
}

BOOL CSmokeGrenade::CanHolster(void)
{
	return !m_flStartThrow;
}

void CSmokeGrenade::Holster(int skiplocal)
{
	m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.5;

	if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		m_pPlayer->pev->weapons &= ~(1 << WEAPON_SMOKEGRENADE);
		DestroyItem();
	}

	m_flStartThrow = 0;
	m_flReleaseThrow = -1;
	EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_WEAPON, "common/null.wav", VOL_NORM, ATTN_NORM);
}

void CSmokeGrenade::PrimaryAttack(void)
{
	if (!m_flStartThrow && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] > 0)
	{
		m_flStartThrow = gpGlobals->time;
		m_flReleaseThrow = 0;
		SendWeaponAnim(SMOKEGRENADE_PULLPIN, UseDecrement() != FALSE);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
	}
}

void CSmokeGrenade::WeaponIdle(void)
{
	if (!m_flReleaseThrow && m_flStartThrow)
		m_flReleaseThrow = gpGlobals->time;

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if (m_flStartThrow)
	{
		m_pPlayer->Radio("%!MRAD_FIREINHOLE", "#Fire_in_the_hole");
		Vector angThrow = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

		if (angThrow.x < 0)
			angThrow.x = -10 + angThrow.x * ((90 - 10) / 90.0);
		else
			angThrow.x = -10 + angThrow.x * ((90 + 10) / 90.0);

		float flVel = (90 - angThrow.x) * 6;

		if (flVel > 750)
			flVel = 750;

		UTIL_MakeVectors(angThrow);
		Vector vecSrc = m_pPlayer->pev->origin + m_pPlayer->pev->view_ofs + gpGlobals->v_forward * 16;
		Vector vecThrow = gpGlobals->v_forward * flVel + m_pPlayer->pev->velocity;
		float time = 1.5;
		CGrenade::ShootSmokeGrenade(m_pPlayer->pev, vecSrc, vecThrow, time, m_usCreateExplo);

		SendWeaponAnim(SMOKEGRENADE_THROW, UseDecrement() != FALSE);
		m_pPlayer->SetAnimation(PLAYER_ATTACK1);
		m_flStartThrow = 0;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.75;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;

		if (!m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
		}
	}
	else if (m_flReleaseThrow > 0)
	{
		m_flStartThrow = 0;

		if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			SendWeaponAnim(SMOKEGRENADE_DEPLOY, UseDecrement() != FALSE);
			m_flReleaseThrow = -1;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT(10, 15);
		}
		else
			RetireWeapon();

		return;
	}

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
	{
		int iAnim;
		float flRand = RANDOM_FLOAT(0, 1);

		if (flRand <= 0.75)
		{
			iAnim = SMOKEGRENADE_IDLE;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + RANDOM_FLOAT(10, 15);
		}
		else
		{
			iAnim = SMOKEGRENADE_IDLE;
			m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 75.0 / 30.0;
		}

		SendWeaponAnim(iAnim, UseDecrement() != FALSE);
	}
}