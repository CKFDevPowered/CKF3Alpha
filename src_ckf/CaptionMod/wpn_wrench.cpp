#include <metahook.h>
#include "plugins.h"
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "weapon.h"
#include "event.h"

void CClientWrench::Precache(void)
{
	m_usFireScript = PRECACHE_EVENT(1, "events/ckf_melee.sc");
}

BOOL CClientWrench::Deploy(void)
{
	m_iShotsFired = 0;

	return GroupDeploy("models/CKF_III/v_wrench.mdl", "models/CKF_III/wp_group_rf.mdl", WRENCH_DRAW, 0, 0, "knife");
}

void CClientWrench::Holster(void)
{
	m_bMeleeAttack = FALSE;
}

void CClientWrench::PrimaryAttack(void)
{
	m_flMeleeAttack = UTIL_WeaponTimeBase() + 0.3;
	m_iMeleeCrit = g_Player.GetCriticalFire(2, g_Player.random_seed);
	m_bMeleeAttack = TRUE;
	SendWeaponAnim(WRENCH_SLASH1 + (m_iShotsFired++) % 2);
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.8;
}

void CClientWrench::Swing(void)
{
	PLAYBACK_EVENT_FULL(FEV_NOTHOST, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, m_iId, 0, (m_iMeleeCrit >= 2) ? TRUE : FALSE, 0);
}