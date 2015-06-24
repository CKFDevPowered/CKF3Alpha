#include <metahook.h>
#include "plugins.h"
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "weapon.h"
#include "event.h"
#include "util.h"

void CClientFlamethrower::Precache(void)
{
	m_usFireScript = PRECACHE_EVENT(1, "events/ckf_flamethrower.sc");
}

BOOL CClientFlamethrower::Deploy(void)
{
	m_iAmmoConsumption = 0;
	m_iState = STATE_FLAMETHROWER_IDLE;
	
	return GroupDeploy("models/CKF_III/v_flamethrower.mdl", "models/CKF_III/wp_group_2bone.mdl", FLAMETHROWER_DRAW, 0, 0, "shotgun");
}

void CClientFlamethrower::Holster(void)
{
	m_iState = STATE_FLAMETHROWER_IDLE;
	gEngfuncs.Con_DPrintf("idle holster\n");
	PLAYBACK_EVENT_FULL(FEV_GLOBAL | FEV_NOTHOST, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, STATE_FLAMETHROWER_IDLE, 0, 0, 0);
}

void CClientFlamethrower::PrimaryAttack(void)
{
	if (m_iAmmo <= 0)
	{
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;
		if(m_iState != STATE_FLAMETHROWER_IDLE)
		{
			//gEngfuncs.Con_DPrintf("idle ammo\n");
			m_iState = STATE_FLAMETHROWER_IDLE;
			PLAYBACK_EVENT_FULL(FEV_NOTHOST, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, m_iState, 0, 0, 0);
			SendWeaponAnim(FLAMETHROWER_IDLE);
		}
		return;
	}

	if (g_flTraceDistance < 54)
	{
		m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;
		if(m_iState != STATE_FLAMETHROWER_IDLE)
		{
			//gEngfuncs.Con_DPrintf("idle ammo\n");
			m_iState = STATE_FLAMETHROWER_IDLE;
			PLAYBACK_EVENT_FULL(FEV_NOTHOST, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, m_iState, 0, 0, 0);
			SendWeaponAnim(FLAMETHROWER_IDLE);
		}
		return;
	}

	int iCrit = g_Player.GetCriticalFire(1, g_Player.random_seed);

	m_iAmmoConsumption = (m_iAmmoConsumption + 1) % 2;

	if(m_iAmmoConsumption == 1)
		m_iAmmo --;

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.04;
	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.15;

	if(m_iState == STATE_FLAMETHROWER_IDLE)
	{
		m_iState = STATE_FLAMETHROWER_START;
		//gEngfuncs.Con_DPrintf("idle to start\n");
		m_flState = UTIL_WeaponTimeBase() + 1.0;
		PLAYBACK_EVENT_FULL(FEV_NOTHOST, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, m_iState, 0, 0, 0);
		SendWeaponAnim(FLAMETHROWER_FIRE);
	}
	else if(m_iState >= STATE_FLAMETHROWER_START && UTIL_WeaponTimeBase() > m_flState)
	{
		m_iState = (iCrit >= 2) ? STATE_FLAMETHROWER_LOOP_CRIT : STATE_FLAMETHROWER_LOOP;
		//gEngfuncs.Con_DPrintf("start/loop to loop\n");
		m_flState = UTIL_WeaponTimeBase() + 3.5;
		PLAYBACK_EVENT_FULL(FEV_NOTHOST, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, m_iState, g_iTeam, 0, 0);
		SendWeaponAnim(FLAMETHROWER_FIRE);
	}
	else if(iCrit >= 2 && m_iState == STATE_FLAMETHROWER_LOOP)
	{
		m_iState = STATE_FLAMETHROWER_LOOP_CRIT;
		//gEngfuncs.Con_DPrintf("loop to loopcrit\n");
		m_flState = UTIL_WeaponTimeBase() + 3.5;
		PLAYBACK_EVENT_FULL(FEV_NOTHOST, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, m_iState, g_iTeam, 0, 0);
		SendWeaponAnim(FLAMETHROWER_FIRE);
	}
	else if(iCrit < 2 && m_iState == STATE_FLAMETHROWER_LOOP_CRIT)
	{
		m_iState = STATE_FLAMETHROWER_LOOP;
		//gEngfuncs.Con_DPrintf("loopcrit to loop\n");
		m_flState = UTIL_WeaponTimeBase() + 3.5;
		PLAYBACK_EVENT_FULL(FEV_NOTHOST, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, m_iState, 0, 0, 0);
	}
}

void CClientFlamethrower::SecondaryAttack(void)
{
	if (m_iAmmo < 20)
	{
		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.2;
		m_iState = STATE_FLAMETHROWER_IDLE;
		return;
	}

	m_iAmmo -= 20;

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.75;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.75;

	if (m_iClip)
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 2.5;
	else
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 0.80;

	m_iState = STATE_FLAMETHROWER_IDLE;
	//gEngfuncs.Con_DPrintf("airblast\n");
	PLAYBACK_EVENT_FULL(FEV_NOTHOST, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, STATE_FLAMETHROWER_AIRBLAST, 0, 0, 0);	
	SendWeaponAnim(FLAMETHROWER_AIRBLAST);
}

void CClientFlamethrower::ItemPostFrame(void)
{
	CClientWeapon::ItemPostFrame();

	/*if(m_iState == STATE_FLAMETHROWER_IDLE || *cls_viewmodel_sequence == FLAMETHROWER_FIRE)
	{
		PLAYBACK_EVENT_FULL(FEV_NOTHOST, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, STATE_FLAMETHROWER_IDLE, 0, 0, 0);	
		SendWeaponAnim(FLAMETHROWER_IDLE);
	}*/
}

void CClientFlamethrower::WeaponIdle(void)
{
	ResetEmptySound();

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	if(m_iState != STATE_FLAMETHROWER_IDLE)
	{
		m_iState = STATE_FLAMETHROWER_IDLE;
		//gEngfuncs.Con_DPrintf("idle not attack\n");
		PLAYBACK_EVENT_FULL(FEV_NOTHOST, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, STATE_FLAMETHROWER_IDLE, 0, 0, 0);
		SendWeaponAnim(FLAMETHROWER_IDLE);
	}
}