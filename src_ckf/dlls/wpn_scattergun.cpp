#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

enum scattergun_e
{
	SCATTERGUN_IDLE1,
	SCATTERGUN_START_RELOAD,
	SCATTERGUN_RELOAD,
	SCATTERGUN_AFTER_RELOAD,
	SCATTERGUN_DRAW,
	SCATTERGUN_SHOOT
};

LINK_ENTITY_TO_CLASS(weapon_scattergun, CScattergun);

void CScattergun::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_scattergun");

	Precache();
	m_iId = WEAPON_SCATTERGUN;
	SET_MODEL(ENT(pev), "models/CKF_III/wp_group_rf.mdl");
	pev->sequence = 3;
	m_iDefaultAmmo = SCATTERGUN_DEFAULT_GIVE;
	FallInit();
}

void CScattergun::Precache(void)
{
	PRECACHE_MODEL("models/CKF_III/v_scattergun.mdl");
	PRECACHE_MODEL("models/CKF_III/wp_group_rf.mdl");

	m_iShellId = PRECACHE_MODEL("models/rshell.mdl");

	PRECACHE_SOUND("CKF_III/scattergun_shoot.wav");
	PRECACHE_SOUND("CKF_III/scattergun_reload.wav");

	m_usFireScript = PRECACHE_EVENT(1, "events/ckf_scattergun.sc");
}

int CScattergun::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "PrimaryAmmo";
	p->iMaxAmmo1 = _SCATTERGUN_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = SCATTERGUN_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 0;
	p->iId = m_iId = WEAPON_SCATTERGUN;
	p->iFlags = 0;
	p->iWeight = SCATTERGUN_WEIGHT;

	return 1;
}

BOOL CScattergun::Deploy(void)
{
	m_fInSpecialReload = 0;

	return GroupDeploy("models/CKF_III/v_scattergun.mdl", "models/CKF_III/wp_group_rf.mdl", SCATTERGUN_DRAW, 0, 0, "shotgun", 9);
}

void CScattergun::PrimaryAttack(void)
{
	float flSpread = 0.14;
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

	int iTracerColor = (iCrit >= 2) ? m_pPlayer->m_iTeam : 0;//Draw

	if(!m_iShotsFired)
	{
		m_pPlayer->CKFFireBullets(vecSrc, gpGlobals->v_forward, 0.0, 8192, BULLET_PLAYER_TF2, 6, iCrit, m_pPlayer->pev, m_pPlayer->random_seed, FALSE);
	}
	int iMaxBurst = (m_iShotsFired) ? 10 : 9;

	for(int iBurst = 0; iBurst < iMaxBurst; iBurst ++)
	{
		Vector vecDst = m_pPlayer->CKFFireBullets(vecSrc, gpGlobals->v_forward, flSpread, 8192, BULLET_PLAYER_TF2, 6, iCrit, m_pPlayer->pev, m_pPlayer->random_seed+iBurst, FALSE);
	}

	ViewAnglesForPlayBack(vecAngles);
	PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFireScript, 0, vecSrc, vecAngles, flSpread, 0, iTracerColor, m_pPlayer->random_seed, (!m_iShotsFired) ? TRUE : FALSE, 0);

	ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.64;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.64;

	if (m_iClip)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5;
	else
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.625;

	m_flDecreaseShotsFired = m_flNextPrimaryAttack + 0.2;
	if(m_iShotsFired < 5) m_iShotsFired ++;

	m_fInSpecialReload = 0;

	//Do punch angle client-side
	//m_pPlayer->pev->punchangle.x -= UTIL_SharedRandomLong(m_pPlayer->random_seed + 1, 4, 6);

	m_pPlayer->m_flEjectBrass = gpGlobals->time + 0.45;

	SendWeaponAnim(SCATTERGUN_SHOOT, UseDecrement() != FALSE);//Send weapon anim before tracer
	
	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
}

void CScattergun::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0 || m_iClip == SCATTERGUN_MAX_CLIP)
		return;

	if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
		return;

	if (!m_fInSpecialReload)
	{
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
		SendWeaponAnim(SCATTERGUN_START_RELOAD, UseDecrement() != FALSE);

		m_fInSpecialReload = 1;
		m_pPlayer->m_flNextAttack = UTIL_WeaponTimeBase() + 0.20;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.20;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.20;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.20;
	}
	else if (m_fInSpecialReload == 1)
	{
		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_fInSpecialReload = 2;

		//EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "CKF_III/scattergun_reload.wav", VOL_NORM, ATTN_NORM, 0, 85 + RANDOM_LONG(0, 31));

		SendWeaponAnim(SCATTERGUN_RELOAD, UseDecrement() != FALSE);

		m_flNextReload = UTIL_WeaponTimeBase() + 0.56;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.56;
	}
	else
	{
		m_iClip++;
		m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
		m_fInSpecialReload = 1;
	}
}

void CScattergun::WeaponIdle(void)
{
	ResetEmptySound();

	if(m_iShotsFired && UTIL_WeaponTimeBase() > m_flDecreaseShotsFired)
	{
		m_iShotsFired --;
		m_flDecreaseShotsFired = UTIL_WeaponTimeBase() + 0.25;
	}

	if (m_flPumpTime && m_flPumpTime < UTIL_WeaponTimeBase())
		m_flPumpTime = 0;

	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		if (!m_iClip && !m_fInSpecialReload && m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
		{
			Reload();
		}
		else if (m_fInSpecialReload)
		{
			if (m_iClip == SCATTERGUN_MAX_CLIP || !m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType])
			{
				SendWeaponAnim(SCATTERGUN_AFTER_RELOAD, UseDecrement() != FALSE);

				m_fInSpecialReload = 0;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;
			}
			else
				Reload();
		}
		else
			SendWeaponAnim(SCATTERGUN_IDLE1, UseDecrement() != FALSE);
	}
}
