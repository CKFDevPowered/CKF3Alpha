#include <metahook.h>
#include "plugins.h"
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "weapon.h"
#include "event.h"

void CClientStickyLauncher::Precache(void)
{
	m_usFireScript = PRECACHE_EVENT(1, "events/ckf_launcher.sc");
}

BOOL CClientStickyLauncher::Deploy(void)
{
	m_fInSpecialReload = 0;
	m_iMaxDeploy = 8;
	
	return GroupDeploy("models/CKF_III/v_stickylauncher.mdl", "models/CKF_III/wp_group_rf.mdl", STICKYLAUNCHER_DRAW, 0, 0, "onehanded");
}

void CClientStickyLauncher::Holster(void)
{
	StopCharge();
}

void CClientStickyLauncher::StopCharge(void)
{
	if(m_iWeaponState & WEAPONSTATE_CHARGING)
	{
		PLAYBACK_EVENT_FULL(FEV_GLOBAL | FEV_NOTHOST, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, m_iId, 1, 0, 0);
	}
	m_iWeaponState &= ~WEAPONSTATE_CHARGING;
	m_fCharge = 0;
}

void CClientStickyLauncher::ItemPostFrame(void)
{
	if(!m_iClip && (m_iWeaponState & WEAPONSTATE_CHARGING) )
	{
		StopCharge();
	}
	if(m_iWeaponState & WEAPONSTATE_CHARGING)
	{
		if(g_Player.pev.button & IN_ATTACK)
		{
			m_fCharge = min(max((UTIL_WeaponTimeBase() + 4.0 - m_fChargeTimer) * (100.0 / 4.0), 0), 100);
		}
		if(!(g_Player.pev.button & IN_ATTACK))
		{
			StickyLauncherFire();
		}
		if(m_fCharge >= 100)
		{
			StickyLauncherFire();
		}
	}

	CClientWeapon::ItemPostFrame();
}

void CClientStickyLauncher::PrimaryAttack(void)
{
	if(m_iClip && !(m_iWeaponState & WEAPONSTATE_CHARGING))
	{
		//EMIT_SOUND(ENT(pev), CHAN_WEAPON, "CKF_III/stickylauncher_charge.wav", 1.0, 1.0);
		PLAYBACK_EVENT_FULL(FEV_NOTHOST, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, m_iId, 2, 0, 0);
		m_fChargeTimer = UTIL_WeaponTimeBase() + 4.0;
		m_fCharge = 0;
		m_iWeaponState |= WEAPONSTATE_CHARGING;
	}
}

void CClientStickyLauncher::StickyLauncherFire(void)
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

	if(m_iStickyNum >= m_iMaxDeploy)
	{
		//do nothing
	}
	else
	{
		//m_iStickyNum ++;
	}

	HUD_PlaybackEvent(FEV_NOTHOST, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, m_iId, 0, (iCrit >= 2) ? TRUE : FALSE, 0);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.6;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase();

	if (m_iClip)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5;
	else
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.80;

	m_fInSpecialReload = 0;

	m_iWeaponState &= ~WEAPONSTATE_CHARGING;
	m_fCharge = 0;

	SendWeaponAnim(STICKYLAUNCHER_SHOOT);
}

void CClientStickyLauncher::SecondaryAttack(void)
{
	
}

void CClientStickyLauncher::Reload(void)
{
	if (m_iAmmo <= 0 || m_iClip == STICKY_MAX_CLIP)
		return;

	if (m_flNextPrimaryAttack > UTIL_WeaponTimeBase())
		return;

	if((m_iWeaponState & WEAPONSTATE_CHARGING))
		return;

	if (!m_fInSpecialReload)
	{
		SendWeaponAnim(STICKYLAUNCHER_START_RELOAD);

		m_fInSpecialReload = 1;
		g_Player.m_flNextAttack = UTIL_WeaponTimeBase() + 0.42;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.42;
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.42;
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.42;
	}
	else if (m_fInSpecialReload == 1)
	{
		if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
			return;

		m_fInSpecialReload = 2;

		SendWeaponAnim(STICKYLAUNCHER_RELOAD);

		m_flNextReload = UTIL_WeaponTimeBase() + 0.67;
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.67;
	}
	else
	{
		m_iClip++;
		m_iAmmo--;
		m_fInSpecialReload = 1;
	}
}

void CClientStickyLauncher::WeaponIdle(void)
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
			if (m_iClip == STICKY_MAX_CLIP || !m_iAmmo)
			{
				SendWeaponAnim(STICKYLAUNCHER_AFTER_RELOAD);

				m_fInSpecialReload = 0;
				m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.5;
			}
			else
				Reload();
		}
		else
			SendWeaponAnim(STICKYLAUNCHER_IDLE);
	}
}