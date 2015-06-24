#include <metahook.h>
#include "plugins.h"
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "weapon.h"
#include "event.h"

void CClientShotgun::Precache(void)
{
	m_usFireScript = PRECACHE_EVENT(1, "events/ckf_shotgun.sc");
}

BOOL CClientShotgun::Deploy(void)
{
	m_bIsPrimary = (g_iClass == CLASS_ENGINEER) ? TRUE : FALSE;

	m_fInSpecialReload = 0;

	//viewmodel body is set by WeaponInfo
	return GroupDeploy("models/CKF_III/v_shotgunall.mdl", "models/CKF_III/wp_group_rf.mdl", SHOTGUN_DRAW, 0, 0, (m_bIsPrimary) ? "shotgun" : "onehanded");
}

void CClientShotgun::PrimaryAttack(void)
{
	float flSpread = 0.10;
	if(m_iShotsFired) flSpread *= min(1.0+m_iShotsFired/10, 1.5);

	if (m_iClip <= 0)
	{
		Reload();

		if (!m_iClip)
			PlayEmptySound();

		return;
	}

	int iCrit = g_Player.GetCriticalFire(0, g_Player.random_seed);

	m_iClip--;


	//Prepare to Draw Tracer
	int iTracerColor = (iCrit >= 2) ? g_iTeam : 0;//Draw

	PLAYBACK_EVENT_FULL( FEV_NOTHOST, NULL, m_usFireScript, 0, NULL, NULL, flSpread, 0, iTracerColor, g_Player.random_seed, (!m_iShotsFired) ? TRUE : FALSE, 0 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.625;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.625;

	if (m_iClip)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5;
	else
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.625;

	m_flDecreaseShotsFired = m_flNextPrimaryAttack + 0.2;
	if(m_iShotsFired < 5) m_iShotsFired ++;

	m_fInSpecialReload = 0;

	m_flDecreaseShotsFired = UTIL_WeaponTimeBase() + 0.25;

	SendWeaponAnim(SHOTGUN_SHOOT);
}

void CClientShotgun::Reload(void)
{
	if (m_iAmmo <= 0 || m_iClip == SHOTGUN_MAX_CLIP)
		return;

	if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
		return;

	if (!m_fInSpecialReload)
	{
		SendWeaponAnim(SHOTGUN_START_RELOAD);

		m_fInSpecialReload = 1;
		g_Player.m_flNextAttack = UTIL_WeaponTimeBase() + 0.50;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.50;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.50;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.50;
	}
	else if (m_fInSpecialReload == 1)
	{
		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_fInSpecialReload = 2;

		SendWeaponAnim(SHOTGUN_RELOAD);

		m_flNextReload = UTIL_WeaponTimeBase() + 0.50;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.50;
	}
	else
	{
		m_iClip++;
		m_iAmmo--;
		m_fInSpecialReload = 1;
	}
}
void CClientShotgun::WeaponIdle(void)
{
	ResetEmptySound();

	if(m_iShotsFired && UTIL_WeaponTimeBase() > m_flDecreaseShotsFired)
	{
		m_iShotsFired --;
		m_flDecreaseShotsFired = UTIL_WeaponTimeBase() + 0.25;
	}

	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		if (!m_iClip && !m_fInSpecialReload && m_iAmmo)
		{
			Reload();
		}
		else if (m_fInSpecialReload)
		{
			if (m_iClip == SHOTGUN_MAX_CLIP || !m_iAmmo)
			{
				SendWeaponAnim(SHOTGUN_AFTER_RELOAD);

				m_fInSpecialReload = 0;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;
			}
			else
				Reload();
		}
		else
			SendWeaponAnim(SHOTGUN_IDLE1);
	}
}
