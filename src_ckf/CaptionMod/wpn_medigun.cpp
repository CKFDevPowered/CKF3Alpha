#include <metahook.h>
#include "plugins.h"
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "weapon.h"
#include "event.h"

void CClientMedigun::Precache(void)
{
	m_usFireScript = PRECACHE_EVENT(1, "events/ckf_medigun.sc");
}

BOOL CClientMedigun::Deploy(void)
{
	return GroupDeploy("models/CKF_III/v_medigun.mdl", "models/CKF_III/wp_group_rf.mdl", MEDIGUN_DRAW, 0, 0, "shotgun");
}

void CClientMedigun::PrimaryAttack(void)
{
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;

	if(!m_bDelayedFire)
	{
		SendWeaponAnim(MEDIGUN_HEAL);
		PLAYBACK_EVENT_FULL(FEV_NOTHOST, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0.0f, 0.0f, 0, 0, FALSE, FALSE);
		m_bDelayedFire = TRUE;
	}
}

void CClientMedigun::WeaponIdle(void)
{
	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;
		SendWeaponAnim(MEDIGUN_IDLE1);
	}
}