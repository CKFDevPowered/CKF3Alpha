#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

enum shotgun_e
{
	SHOTGUN_IDLE1,
	SHOTGUN_START_RELOAD,
	SHOTGUN_RELOAD,
	SHOTGUN_AFTER_RELOAD,
	SHOTGUN_DRAW,
	SHOTGUN_SHOOT	
};

LINK_ENTITY_TO_CLASS(weapon_shotgun, CShotgun);

void CShotgun::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_shotgun");

	Precache();
	m_iId = WEAPON_SHOTGUN;
	m_bIsPrimary = FALSE;
	SET_MODEL(ENT(pev), "models/CKF_III/wp_group_rf.mdl");
	pev->sequence = 1;

	m_iDefaultAmmo = SHOTGUN_DEFAULT_GIVE;
	FallInit();
}

void CShotgun::Precache(void)
{
	m_iShellId = PRECACHE_MODEL("models/rshell.mdl");

	PRECACHE_MODEL("models/CKF_III/v_shotgunall.mdl");
	PRECACHE_MODEL("models/CKF_III/wp_group_rf.mdl");
	PRECACHE_SOUND("CKF_III/shotgun_shoot.wav");
	PRECACHE_SOUND("CKF_III/shotgun_reload.wav");

	m_usFireScript = PRECACHE_EVENT(1, "events/ckf_shotgun.sc");
}

int CShotgun::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	
	p->iMaxAmmo1 = _SHOTGUN_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = SHOTGUN_MAX_CLIP;
	p->iSlot = 0;
	p->iPosition = 0;
	p->iId = m_iId = WEAPON_SHOTGUN;
	p->iFlags = 0;
	p->iWeight = SHOTGUN_WEIGHT;

	return 1;
}

const char *CShotgun::pszAmmo1(void)
{
	return (!m_bIsPrimary) ? "SecondaryAmmo" : "PrimaryAmmo";
}

int CShotgun::iMaxAmmo1(void)
{
	return _SHOTGUN_MAX_CARRY;
}

int CShotgun::iSlot(void)
{
	return (!m_bIsPrimary) ? 1 : 0;
}

int CShotgun::iWeight(void)
{
	return (!m_bIsPrimary) ? SHOTGUN_WEIGHT : 30;
}

int CShotgun::iBody(void)
{
	int iViewBody = 0;
	if(m_pPlayer)
	{	
		if(m_pPlayer->m_iClass == CLASS_SOLDIER)
			iViewBody = 1;
		else if(m_pPlayer->m_iClass == CLASS_PYRO)
			iViewBody = 2;
		else if(m_pPlayer->m_iClass == CLASS_ENGINEER)
			iViewBody = 3;
	}
	return iViewBody;
}

BOOL CShotgun::Deploy(void)
{
	m_fInSpecialReload = 0;
	
	return GroupDeploy("models/CKF_III/v_shotgunall.mdl", "models/CKF_III/wp_group_rf.mdl", SHOTGUN_DRAW, 0, 0, (m_bIsPrimary) ? "shotgun" : "onehanded", 3);
}

void CShotgun::PrimaryAttack(void)
{
	float flSpread = 0.10;
	if(m_iShotsFired) flSpread *= min(1.0+m_iShotsFired/10.0f, 1.5);

	if (m_iClip <= 0)
	{
		Reload();

		if (!m_iClip)
			PlayEmptySound();

		return;
	}

	int iCrit = m_pPlayer->GetCriticalFire(0, m_pPlayer->random_seed);

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
	m_iClip--;
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

	Vector vecAngles = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;
	UTIL_MakeVectors(vecAngles);
	Vector vecSrc = m_pPlayer->GetGunPosition();

	ClearMultiDamage();

	if(!m_iShotsFired) m_pPlayer->CKFFireBullets(vecSrc, gpGlobals->v_forward, 0.0, 8192, BULLET_PLAYER_TF2, 6, iCrit, m_pPlayer->pev, m_pPlayer->random_seed, FALSE);
	int iMaxBurst = (m_iShotsFired) ? 10 : 9;

	for(int iBurst = 0; iBurst < iMaxBurst; iBurst ++)
	{
		Vector vecDst = m_pPlayer->CKFFireBullets(vecSrc, gpGlobals->v_forward, flSpread, 8192, BULLET_PLAYER_TF2, 6, iCrit, m_pPlayer->pev, m_pPlayer->random_seed+iBurst, FALSE);
	}

	//Prepare to Draw Tracer
	int iTracerColor = (iCrit >= 2) ? m_pPlayer->m_iTeam : 0;//Draw

	ViewAnglesForPlayBack(vecAngles);
	PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFireScript, 0, vecSrc, vecAngles, flSpread, 0, iTracerColor, m_pPlayer->random_seed, (!m_iShotsFired) ? TRUE : FALSE, 0);

	ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.625;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.625;

	if (m_iClip)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5;
	else
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.625;

	m_flDecreaseShotsFired = m_flNextPrimaryAttack + 0.2;
	if(m_iShotsFired < 5) m_iShotsFired ++;

	m_fInSpecialReload = 0;

	//m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 2, 3);

	m_pPlayer->m_flEjectBrass = gpGlobals->time + 0.45;

	SendWeaponAnim(SHOTGUN_SHOOT, UseDecrement() != FALSE);

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
}

void CShotgun::SecondaryAttack(void)
{
	m_pPlayer->Build_Undeploy();
}

void CShotgun::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == SHOTGUN_MAX_CLIP)
		return;

	if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
		return;

	if (!m_fInSpecialReload)
	{
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
		SendWeaponAnim(SHOTGUN_START_RELOAD, UseDecrement() != FALSE);

		m_fInSpecialReload = 1;
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.50;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.50;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.50;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.50;
	}
	else if (m_fInSpecialReload == 1)
	{
		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_fInSpecialReload = 2;

		EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "CKF_III/shotgun_reload.wav", VOL_NORM, ATTN_NORM, 0, 85 + RANDOM_LONG(0, 31));

		SendWeaponAnim(SHOTGUN_RELOAD, UseDecrement() != FALSE);

		m_flNextReload = UTIL_WeaponTimeBase() + 0.50;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.50;
	}
	else
	{
		m_iClip++;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
		m_fInSpecialReload = 1;
	}
}

void CShotgun::WeaponIdle(void)
{
	ResetEmptySound();
	//m_pPlayer->GetAutoaimVector(AUTOAIM_5DEGREES);

	if(m_iShotsFired && UTIL_WeaponTimeBase() > m_flDecreaseShotsFired)
	{
		m_iShotsFired --;
		m_flDecreaseShotsFired = UTIL_WeaponTimeBase() + 0.25;
	}

	//if (m_flPumpTime && m_flPumpTime < UTIL_WeaponTimeBase())
	//	m_flPumpTime = 0;

	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		if (!m_iClip && !m_fInSpecialReload && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			Reload();
		}
		else if (m_fInSpecialReload)
		{
			if (m_iClip == SHOTGUN_MAX_CLIP || !m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			{
				SendWeaponAnim(SHOTGUN_AFTER_RELOAD, UseDecrement() != FALSE);

				m_fInSpecialReload = 0;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;
			}
			else
				Reload();
		}
		else
			SendWeaponAnim(SHOTGUN_IDLE1, UseDecrement() != FALSE);
	}
}

extern int gmsgWeaponList;

void CShotgun::PreAttachPlayer(CBasePlayer *pPlayer)
{
	m_bIsPrimary = (pPlayer->m_iClass == CLASS_ENGINEER) ? TRUE : FALSE;
}