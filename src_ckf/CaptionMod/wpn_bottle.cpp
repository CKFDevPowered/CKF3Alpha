#include <metahook.h>
#include "plugins.h"
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "weapon.h"
#include "event.h"

void CClientBottle::Precache(void)
{
	m_usFireScript = PRECACHE_EVENT(1, "events/ckf_melee.sc");
}

BOOL CClientBottle::Deploy(void)
{
	m_iShotsFired = 0;

	return GroupDeploy("models/CKF_III/v_bottle.mdl", "models/CKF_III/wp_group_rf.mdl", BOTTLE_DRAW, 0, 0, "knife");
}

void CClientBottle::Holster(void)
{
	m_bMeleeAttack = FALSE;
}

void CClientBottle::PrimaryAttack(void)
{
	m_flMeleeAttack = UTIL_WeaponTimeBase() + 0.3;
	m_iMeleeCrit = g_Player.GetCriticalFire(2, g_Player.random_seed);
	m_bMeleeAttack = TRUE;

	int iAnimBase = (m_bBroken) ? BOTTLE_SLASH1_BROKEN : BOTTLE_SLASH1;
	SendWeaponAnim(iAnimBase + (m_iShotsFired++) % 2);

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.8;
}

void CClientBottle::Swing(void)
{
	PLAYBACK_EVENT_FULL(FEV_NOTHOST, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, m_iId, 0, (m_iMeleeCrit >= 2) ? TRUE : FALSE, 0);
}