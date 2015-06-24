#include <metahook.h>
#include "plugins.h"
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "weapon.h"
#include "event.h"

void CClientGrenadeLauncher::Precache(void)
{
	m_usFireScript = PRECACHE_EVENT(1, "events/ckf_launcher.sc");
}

BOOL CClientGrenadeLauncher::Deploy(void)
{
	m_fInSpecialReload = 0;
	
	return GroupDeploy("models/CKF_III/v_grenadelauncher.mdl", "models/CKF_III/wp_group_2bone.mdl", GRENADELAUNCHER_DRAW, 0, 0, "shotgun");
}

void CClientGrenadeLauncher::PrimaryAttack(void)
{
	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
		}

		return;
	}

	int iCrit = g_Player.GetCriticalFire(0, g_Player.random_seed);

	m_iClip--;

	SendWeaponAnim(ROCKETLAUNCHER_SHOOT);

	PLAYBACK_EVENT_FULL(FEV_NOTHOST, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, m_iId, 0, (iCrit >= 2) ? TRUE : FALSE, 0);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.6;

	if (m_iClip)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5;
	else
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.80;

	m_fInSpecialReload = 0;
}

void CClientGrenadeLauncher::SecondaryAttack(void)
{
	
}

void CClientGrenadeLauncher::Reload(void)
{
	if (m_iAmmo <= 0 || m_iClip == GRENADE_MAX_CLIP)
		return;

	if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
		return;

	if (!m_fInSpecialReload)
	{
		SendWeaponAnim(GRENADELAUNCHER_START_RELOAD);

		m_fInSpecialReload = 1;
		g_Player.m_flNextAttack = UTIL_WeaponTimeBase() + 0.64;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.64;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.64;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.64;
	}
	else if (m_fInSpecialReload == 1)
	{
		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_fInSpecialReload = 2;

		SendWeaponAnim(GRENADELAUNCHER_RELOAD);

		m_flNextReload = UTIL_WeaponTimeBase() + 0.60;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.60;
	}
	else
	{
		m_iClip++;
		m_iAmmo--;
		m_fInSpecialReload = 1;
	}
}

void CClientGrenadeLauncher::WeaponIdle(void)
{
	ResetEmptySound();

	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		if (!m_iClip && !m_fInSpecialReload && m_iAmmo)
		{
			Reload();
		}
		else if (m_fInSpecialReload)
		{
			if (m_iClip == GRENADE_MAX_CLIP || !m_iAmmo)
			{
				SendWeaponAnim(GRENADELAUNCHER_AFTER_RELOAD);

				m_fInSpecialReload = 0;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
			}
			else
				Reload();
		}
		else
			SendWeaponAnim(GRENADELAUNCHER_IDLE);
	}
}