#include <metahook.h>
#include "plugins.h"
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "weapon.h"
#include "event.h"

void CClientSniperifle::Precache(void)
{
	m_usFireScript = PRECACHE_EVENT(1, "events/ckf_sniperifle.sc");
}

BOOL CClientSniperifle::Deploy(void)
{
	m_fCharge = 0;
	m_iWeaponState &= ~WEAPONSTATE_CHARGING;
	m_fSpotActive = 0;

	return GroupDeploy("models/CKF_III/v_sniperifle.mdl", "models/CKF_III/wp_group_rf.mdl", SNIPERIFLE_DRAW, 0, 0, "shotgun");
}

BOOL CClientSniperifle::CanHolster(void)
{
	if(m_iResumeZoom == 1)
		return FALSE;
	return TRUE;
}

void CClientSniperifle::Holster(void)
{
	g_Player.pev.fov = g_iDefaultFOV;

	m_iResumeZoom = 0;
	m_fCharge = 0;
	m_fSpotActive = 0;
	m_iWeaponState &= ~WEAPONSTATE_CHARGING;

	g_Player.ResetMaxSpeed();
}

void CClientSniperifle::StartZoom(void)
{
	g_Player.pev.fov = 32 * g_iDefaultFOV / 90;
	m_fChargeTimer = UTIL_WeaponTimeBase() + 4.0;
	m_fCharge = 0;
	
	m_iWeaponState |= WEAPONSTATE_CHARGING;
	m_fSpotActive = 1;

	g_Player.ResetMaxSpeed();
	PLAYBACK_EVENT_FULL(FEV_NOTHOST, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, 0, 1, 0, 0);

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5f;
	m_iResumeZoom = 0;
}

void CClientSniperifle::StopZoom(void)
{
	g_Player.pev.fov = g_iDefaultFOV;
	m_fCharge = 0;

	m_iWeaponState &= ~WEAPONSTATE_CHARGING;
	m_fSpotActive = 0;

	g_Player.ResetMaxSpeed();
	PLAYBACK_EVENT_FULL(FEV_NOTHOST, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, 0, 1, 0, 0);

	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5f;
	m_iResumeZoom = 0;
}

void CClientSniperifle::PrimaryAttack(void)
{
	int iCrit = g_Player.GetCriticalFire(-1, g_Player.random_seed);

	if (m_iWeaponState & WEAPONSTATE_CHARGING)
	{
		m_iResumeZoom = 1;
		m_flNextReload = UTIL_WeaponTimeBase() + 0.35;
	}

	if (m_iAmmo <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
		}
		return;
	}

	m_iAmmo--;

	PLAYBACK_EVENT_FULL(FEV_NOTHOST, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, iCrit, 0, 0, 0);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 1.5;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.9;

	m_fSpotActive = 0;
	m_iWeaponState &= ~WEAPONSTATE_CHARGING;

	SendWeaponAnim(SNIPERIFLE_FIRE);
}

void CClientSniperifle::SecondaryAttack(void)
{
	if(m_iResumeZoom)
		return;

	if ( !(m_iWeaponState & WEAPONSTATE_CHARGING) && (g_Player.pev.flags & FL_ONGROUND) )
	{
		StartZoom();
	}
	else if ( (m_iWeaponState & WEAPONSTATE_CHARGING) )
	{
		StopZoom();
	}
}

void CClientSniperifle::ItemPostFrame(void)
{
	if (m_iResumeZoom && m_flNextReload <= UTIL_WeaponTimeBase())
	{
		if (m_iResumeZoom == 1)
		{
			g_Player.pev.fov = g_iDefaultFOV;
			m_fCharge = 0;
			m_flNextReload = m_flNextPrimaryAttack;
			g_Player.ResetMaxSpeed();
			m_iResumeZoom = 2;
			m_iWeaponState &= ~WEAPONSTATE_CHARGING;
		}
		else if (m_iResumeZoom == 2)
		{
			StartZoom();
		}
	}

	if(m_iWeaponState & WEAPONSTATE_CHARGING)
	{
		if((g_Player.pev.oldbuttons & IN_JUMP) && (g_Player.pev.fuser2 > 1300.0f))
		{
			StopZoom();
		}
		else
		{
			if(m_fChargeTimer > UTIL_WeaponTimeBase())
			{
				m_fCharge = (4.0 + UTIL_WeaponTimeBase() - m_fChargeTimer) / 4.0 * 100;;
			}
			else
			{
				m_fCharge = 100;
			}
		}
	}
	CClientWeapon::ItemPostFrame();
}

void CClientSniperifle::WeaponIdle(void)
{
	ResetEmptySound();

	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;
		SendWeaponAnim(SNIPERIFLE_IDLE1);
	}
}

float CClientSniperifle::GetMaxSpeed(void)
{
	if (!(m_iWeaponState & WEAPONSTATE_CHARGING))
		return 1;

	return 0.27;
}