#include <metahook.h>
#include "plugins.h"
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "weapon.h"
#include "event.h"

void CClientRevolver::Precache(void)
{
	m_usFireScript = PRECACHE_EVENT(1, "events/ckf_revolver.sc");
}

BOOL CClientRevolver::Deploy(void)
{
	return GroupDeploy("models/CKF_III/v_revolver.mdl", "models/CKF_III/wp_group_rf.mdl", REVOLVER_DRAW, 0, 0, "shotgun");
}

void CClientRevolver::PrimaryAttack(void)
{
	float flSpread = 0.02;

	if(m_iShotsFired) flSpread *= min(1.0+m_iShotsFired/5.0f, 1.3);

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.58;
		}
		return;
	}

	int iCrit = g_Player.GetCriticalFire(0, g_Player.random_seed);

	m_iClip--;

	int iTracerColor = (iCrit >= 2) ? g_iTeam : 0;//Draw

	PLAYBACK_EVENT_FULL(FEV_NOTHOST, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, flSpread, 0, iTracerColor, g_Player.random_seed, (!m_iShotsFired) ? TRUE : FALSE, 0);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.58f;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.9;

	m_flDecreaseShotsFired = m_flNextPrimaryAttack + 0.4167f;
	if(m_iShotsFired < 3) m_iShotsFired ++;

	SendWeaponAnim(REVOLVER_FIRE);

	m_fInReload = false;
}

void CClientRevolver::Reload(void)
{
	DefaultReload(REVOLVER_MAX_CLIP, REVOLVER_RELOAD, 1.16);
}

void CClientRevolver::WeaponIdle(void)
{
	ResetEmptySound();

	if(m_iShotsFired && UTIL_WeaponTimeBase() > m_flDecreaseShotsFired)
	{
		m_iShotsFired --;
		m_flDecreaseShotsFired = UTIL_WeaponTimeBase() + 0.4167f;
	}

	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;
		SendWeaponAnim(REVOLVER_IDLE1);
	}
}
