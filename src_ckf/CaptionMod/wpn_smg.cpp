#include <metahook.h>
#include "plugins.h"
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "weapon.h"
#include "event.h"

void CClientSMG::Precache(void)
{
	m_usFireScript = PRECACHE_EVENT(1, "events/ckf_smg.sc");
}

BOOL CClientSMG::Deploy(void)
{
	return GroupDeploy("models/CKF_III/v_smg.mdl", "models/CKF_III/wp_group_rf.mdl", SMG_DRAW, 0, 0, "onehanded");
}

void CClientSMG::PrimaryAttack(void)
{
	float flSpread = 0.03;

	if(m_iShotsFired) flSpread *= min(1.0+m_iShotsFired/20.0f, 1.5);

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

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.1;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.9;

	m_flDecreaseShotsFired = m_flNextPrimaryAttack + 0.2;
	if(m_iShotsFired < 5) m_iShotsFired ++;

	SendWeaponAnim(SMG_FIRE);

	m_fInReload = false;
}

void CClientSMG::Reload(void)
{
	DefaultReload(SMG_MAX_CLIP, SMG_RELOAD, 1.4);
}

void CClientSMG::WeaponIdle(void)
{
	ResetEmptySound();

	if(m_iShotsFired && UTIL_WeaponTimeBase() > m_flDecreaseShotsFired)
	{
		m_iShotsFired --;
		m_flDecreaseShotsFired = UTIL_WeaponTimeBase() + 0.25;
	}

	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;
		SendWeaponAnim(SMG_IDLE1);
	}
}