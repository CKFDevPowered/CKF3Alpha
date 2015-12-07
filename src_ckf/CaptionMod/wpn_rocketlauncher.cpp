#include <metahook.h>
#include "plugins.h"
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "weapon.h"
#include "event.h"

void CClientRocketLauncher::Precache(void)
{
	m_usFireScript = PRECACHE_EVENT(1, "events/ckf_launcher.sc");
}

BOOL CClientRocketLauncher::Deploy(void)
{
	m_fInSpecialReload = 0;
	
	return GroupDeploy("models/CKF_III/v_rocketlauncher.mdl", "models/CKF_III/wp_group_rf.mdl", ROCKETLAUNCHER_DRAW, 0, 0, "shotgun");
}

void CClientRocketLauncher::PrimaryAttack(void)
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

	PLAYBACK_EVENT_FULL(FEV_NOTHOST, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, m_iId, 0, (iCrit >= 2) ? TRUE : FALSE, 0);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.8;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.80;

	m_fInSpecialReload = 0;

	SendWeaponAnim(ROCKETLAUNCHER_SHOOT);
}

void CClientRocketLauncher::Reload(void)
{
	if (m_iAmmo <= 0 || m_iClip == ROCKET_MAX_CLIP)
		return;

	if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
		return;

	if (!m_fInSpecialReload)
	{
		SendWeaponAnim(ROCKETLAUNCHER_START_RELOAD);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;

		m_fInSpecialReload = 1;
		m_flNextReload = UTIL_WeaponTimeBase() + 0.50;
	}
}

void CClientRocketLauncher::Reloaded(void)
{
	if (m_iAmmo <= 0 || m_iClip == ROCKET_MAX_CLIP)//out of ammo or full of clip, stop reloading
	{
		SendWeaponAnim(ROCKETLAUNCHER_AFTER_RELOAD);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;
		m_fInSpecialReload = 0;
	}
	else if (m_fInSpecialReload == 2)
	{
		m_iClip++;
		m_iAmmo--;

		m_fInSpecialReload = 1;//go back to start stage
		Reloaded();//have the next try now so weapon anim will be played immediately 
	}
	else
	{
		m_fInSpecialReload = 2;//reloading stage

		SendWeaponAnim(ROCKETLAUNCHER_RELOAD);
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;
		m_flNextReload = UTIL_WeaponTimeBase() + 0.70;		
	}
}

void CClientRocketLauncher::WeaponIdle(void)
{
	ResetEmptySound();

	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;
		SendWeaponAnim(ROCKETLAUNCHER_IDLE);
	}
}