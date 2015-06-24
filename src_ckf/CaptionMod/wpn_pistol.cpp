#include <metahook.h>
#include "plugins.h"
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "weapon.h"
#include "event.h"

void CClientPistol::Precache(void)
{
	m_usFireScript = PRECACHE_EVENT(1, "events/ckf_pistol.sc");
}

BOOL CClientPistol::Deploy(void)
{
	return GroupDeploy((g_iClass == CLASS_SCOUT) ? "models/CKF_III/v_pistol.mdl" : "models/CKF_III/v_pistol_engineer.mdl", "models/CKF_III/wp_group_rf.mdl", PISTOL_DRAW, 0, 0, "onehanded");
}

void CClientPistol::PrimaryAttack(void)
{
	float flSpread = 0.05;

	if(m_iShotsFired) flSpread *= min(1.0+m_iShotsFired/10.0f, 1.5);

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

	m_iClip--;

	int iTracerColor = (iCrit >= 2) ? g_iTeam : 0;//Draw

	PLAYBACK_EVENT_FULL(FEV_NOTHOST, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, flSpread, 0, iTracerColor, g_Player.random_seed, (!m_iShotsFired) ? TRUE : FALSE, 0);

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 1.0f/6;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.9;

	m_flDecreaseShotsFired = m_flNextPrimaryAttack + 1.0f/6;

	if(m_iShotsFired < 5) m_iShotsFired ++;

	SendWeaponAnim(PISTOL_FIRE);
}

void CClientPistol::Reload(void)
{
	if (m_iAmmo <= 0)
		return;

	DefaultReload(PISTOL_MAX_CLIP, PISTOL_RELOAD, (g_iClass == CLASS_SCOUT) ? 1.25 : 1.36);
}

void CClientPistol::WeaponIdle(void)
{
	ResetEmptySound();

	if(m_iShotsFired && UTIL_WeaponTimeBase() > m_flDecreaseShotsFired)
	{
		m_iShotsFired --;
		m_flDecreaseShotsFired = UTIL_WeaponTimeBase() + 0.25;
	}

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;
	SendWeaponAnim(PISTOL_IDLE1);
}