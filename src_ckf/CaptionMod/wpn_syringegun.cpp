#include <metahook.h>
#include "plugins.h"
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "weapon.h"
#include "event.h"

void CClientSyringegun::Precache(void)
{
	m_usFireScript = PRECACHE_EVENT(1, "events/ckf_syringegun.sc");
}

BOOL CClientSyringegun::Deploy(void)
{
	return GroupDeploy("models/CKF_III/v_syringegun.mdl", "models/CKF_III/wp_group_rf.mdl", SYRINGEGUN_DRAW, 0, 0, "onehanded");
}

void CClientSyringegun::PrimaryAttack(void)
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

	int iCrit = g_Player.GetCriticalFire(1, g_Player.random_seed);

	m_iClip --;

	PLAYBACK_EVENT_FULL(FEV_NOTHOST, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, 0, 0, (iCrit >= 2) ? TRUE : FALSE, FALSE);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1f;

	if (m_iClip)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5;
	else
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.80;

	SendWeaponAnim(SYRINGEGUN_SHOOT);
}

void CClientSyringegun::Reload(void)
{
	if (m_iAmmo <= 0)
		return;

	DefaultReload(SYRINGE_MAX_CLIP, SYRINGEGUN_RELOAD, 1.6);
}

void CClientSyringegun::WeaponIdle(void)
{
	ResetEmptySound();

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;
	SendWeaponAnim(SYRINGEGUN_IDLE);
}