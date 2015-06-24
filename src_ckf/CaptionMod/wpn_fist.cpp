#include <metahook.h>
#include "plugins.h"
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "weapon.h"
#include "event.h"

void CClientFist::Precache(void)
{
	m_usFireScript = PRECACHE_EVENT(1, "events/ckf_melee.sc");
}

BOOL CClientFist::Deploy(void)
{
	m_iShotsFired = 0;

	return DefaultDeploy("models/CKF_III/v_fist.mdl", "models/CKF_III/null.mdl", FIST_DRAW, "knife");
}

void CClientFist::Holster(void)
{
	m_bMeleeAttack = FALSE;
}

void CClientFist::PrimaryAttack(void)
{
	m_flMeleeAttack = UTIL_WeaponTimeBase() + 0.2;
	m_iMeleeCrit = g_Player.GetCriticalFire(2, g_Player.random_seed);
	m_bMeleeAttack = TRUE;
	SendWeaponAnim(FIST_SLASH1 + (m_iShotsFired++) % 2);
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.8;
}

void CClientFist::Swing(void)
{
	PLAYBACK_EVENT_FULL(FEV_NOTHOST, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, m_iId, 0, (m_iMeleeCrit >= 2) ? TRUE : FALSE, 0);
}