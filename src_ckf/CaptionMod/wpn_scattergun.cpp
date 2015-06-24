#include <metahook.h>
#include "plugins.h"
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "weapon.h"
#include "event.h"

void CClientScattergun::Precache(void)
{
	m_usFireScript = PRECACHE_EVENT(1, "events/ckf_scattergun.sc");
}

BOOL CClientScattergun::Deploy(void)
{
	m_fInSpecialReload = 0;

	return GroupDeploy("models/CKF_III/v_scattergun.mdl", "models/CKF_III/wp_group_rf.mdl", SCATTERGUN_DRAW, 0, 0, "shotgun");
}

void CClientScattergun::PrimaryAttack(void)
{
	float flSpread = 0.08;

	if(m_iShotsFired) flSpread *= min(1.0+m_iShotsFired/10.0f, 1.5);

	if (m_iClip <= 0)
	{
		Reload();

		if (!m_iClip)
			PlayEmptySound();

		return;
	}

	int iCrit = g_Player.GetCriticalFire(0, g_Player.random_seed);

	m_iClip--;

	int iTracerColor = (iCrit >= 2) ? g_iTeam : 0;//Draw

	PLAYBACK_EVENT_FULL( FEV_NOTHOST, NULL, m_usFireScript, 0, NULL, NULL, flSpread, 0, iTracerColor, g_Player.random_seed, (!m_iShotsFired) ? TRUE : FALSE, 0 );

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.64;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.64;

	if (m_iClip)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5;
	else
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.625;

	m_flDecreaseShotsFired = m_flNextPrimaryAttack + 0.2;
	if(m_iShotsFired < 5) m_iShotsFired ++;

	m_fInSpecialReload = 0;

	m_flDecreaseShotsFired = UTIL_WeaponTimeBase() + 0.25;

	SendWeaponAnim(SCATTERGUN_SHOOT);
}

void CClientScattergun::Reload(void)
{
	if (m_iAmmo <= 0 || m_iClip == SCATTERGUN_MAX_CLIP)
		return;

	if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
		return;

	if (!m_fInSpecialReload)
	{
		SendWeaponAnim(SCATTERGUN_START_RELOAD);

		m_fInSpecialReload = 1;
		g_Player.m_flNextAttack = UTIL_WeaponTimeBase() + 0.20;
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

		SendWeaponAnim(SCATTERGUN_RELOAD);

		m_flNextReload = UTIL_WeaponTimeBase() + 0.56;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.56;
	}
	else
	{
		m_iClip++;
		m_iAmmo--;
		m_fInSpecialReload = 1;
	}
}

void CClientScattergun::WeaponIdle(void)
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
			if (m_iClip == SCATTERGUN_MAX_CLIP || !m_iAmmo)
			{
				SendWeaponAnim(SCATTERGUN_AFTER_RELOAD);

				m_fInSpecialReload = 0;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.5;
			}
			else
				Reload();
		}
		else
			SendWeaponAnim(SCATTERGUN_IDLE1);
	}
}