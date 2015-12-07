#include <metahook.h>
#include "plugins.h"
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "weapon.h"
#include "event.h"

void CClientMinigun::Precache(void)
{
	m_usFireScript = PRECACHE_EVENT(1, "events/ckf_minigun.sc");
}

BOOL CClientMinigun::Deploy(void)
{
	m_iSpin = STATE_MINIGUN_NONE;
	
	return GroupDeploy("models/CKF_III/v_minigun.mdl", "models/CKF_III/wp_group_2bone.mdl", MINIGUN_DRAW, 0, 0, "shotgun");
}

void CClientMinigun::Holster(void)
{
	m_iSpin = STATE_MINIGUN_NONE;
	g_Player.pev.flags &= ~FL_LOCK_DUCK;
	g_Player.pev.flags &= ~FL_LOCK_JUMP;

	PLAYBACK_EVENT_FULL(FEV_GLOBAL, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, STATE_MINIGUN_NONE, 0, 0, 0);
}

BOOL CClientMinigun::CanHolster(void)
{
	if(m_iSpin != STATE_MINIGUN_NONE)
		return FALSE;
	return TRUE;
}

void CClientMinigun::PrimaryAttack(void)
{
	if(m_iSpin < STATE_MINIGUN_SPIN || m_iSpin > STATE_MINIGUN_FIRECRIT)
		return;

	float flSpread = 0.06;
	if(m_iShotsFired)
		flSpread *= min(1.0+m_iShotsFired/20.0f, 1.5);

	if (m_iAmmo <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
		}
		return;
	}

	int iCrit = g_Player.GetCriticalFire(1, g_Player.random_seed);

	m_iAmmo--;

	//Prepare to Draw Tracer
	int iTracerColor = (iCrit >= 2) ? g_iTeam : 0;//Draw

	PLAYBACK_EVENT_FULL(FEV_NOTHOST, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, flSpread, 0, iTracerColor, g_Player.random_seed, (!m_iShotsFired) ? TRUE : FALSE, TRUE);

	//Fire event if necessary
	if(iCrit >= 2 && m_iSpin != STATE_MINIGUN_FIRECRIT)
	{
		m_iSpin = STATE_MINIGUN_FIRECRIT;
		SendWeaponAnim(MINIGUN_SPIN_SHOOT);
		PLAYBACK_EVENT_FULL(FEV_NOTHOST, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, STATE_MINIGUN_FIRECRIT, 0, 0, 0);
	}
	else if(iCrit < 2 && m_iSpin != STATE_MINIGUN_FIRE)
	{
		m_iSpin = STATE_MINIGUN_FIRE;
		SendWeaponAnim(MINIGUN_SPIN_SHOOT);
		PLAYBACK_EVENT_FULL(FEV_NOTHOST, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, STATE_MINIGUN_FIRE, 0, 0, 0);
	}

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.1;

	m_flDecreaseShotsFired = m_flNextPrimaryAttack + 0.2;

	if(m_iShotsFired < 10) m_iShotsFired ++;
}

void CClientMinigun::ItemPostFrame(void)
{
	int iButton = g_Player.pev.button;
	int iOldButtons = g_Player.pev.oldbuttons;

	if((m_iSpin == STATE_MINIGUN_FIRE || m_iSpin == STATE_MINIGUN_FIRECRIT) && (!(iButton & IN_ATTACK) || m_iAmmo <= 0))
	{
		m_iSpin = STATE_MINIGUN_SPIN;
		SendWeaponAnim(MINIGUN_SPIN_IDLE1);
		PLAYBACK_EVENT_FULL(FEV_NOTHOST, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, STATE_MINIGUN_SPIN, 0, 0, 0);
	}
	else if((iButton & IN_ATTACK) || (iButton & IN_ATTACK2))
	{
		switch(m_iSpin)
		{
			case STATE_MINIGUN_NONE:
			{
				if(!(iButton & IN_DUCK) && !(iOldButtons & IN_DUCK) && !g_Player.pev.bInDuck)
				{
					m_iSpin = STATE_MINIGUN_WINDUP;
					m_fSpin = UTIL_WeaponTimeBase() + 0.87;
					SendWeaponAnim(MINIGUN_WIND_UP);
					PLAYBACK_EVENT_FULL(FEV_NOTHOST, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, STATE_MINIGUN_WINDUP, 0, 0, 0);

					g_Player.pev.flags |= FL_LOCK_DUCK;
					g_Player.pev.flags |= FL_LOCK_JUMP;
					g_Player.ResetMaxSpeed();
				}
				break;
			}
			case STATE_MINIGUN_WINDUP:
			{
				if(m_fSpin < UTIL_WeaponTimeBase())
				{
					m_iSpin = STATE_MINIGUN_SPIN;
					SendWeaponAnim(MINIGUN_SPIN_IDLE1);					
					PLAYBACK_EVENT_FULL(FEV_NOTHOST, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, STATE_MINIGUN_SPIN, 0, 0, 0);
				}
				break;
			}
			case STATE_MINIGUN_SPIN:case STATE_MINIGUN_FIRE:case STATE_MINIGUN_FIRECRIT:
			{
				if((iButton & IN_ATTACK) && m_flNextPrimaryAttack < UTIL_WeaponTimeBase())
				{
					PrimaryAttack();
				}
				break;
			}
		}
	}
	else if(m_fSpin < UTIL_WeaponTimeBase() && !(iButton & IN_ATTACK) && !(iButton & IN_ATTACK2))
	{
		switch(m_iSpin)
		{
			case STATE_MINIGUN_SPIN:case STATE_MINIGUN_WINDUP:
			{
				m_iSpin = STATE_MINIGUN_WINDDOWN;
				m_fSpin = UTIL_WeaponTimeBase() + 0.6;
				SendWeaponAnim(MINIGUN_WIND_DOWN);
				PLAYBACK_EVENT_FULL(FEV_NOTHOST, NULL, m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, STATE_MINIGUN_WINDDOWN, 0, 0, 0);
				break;
			}
			case STATE_MINIGUN_WINDDOWN:
			{
				if(m_fSpin < UTIL_WeaponTimeBase())
				{
					m_iSpin = STATE_MINIGUN_NONE;
					SendWeaponAnim(MINIGUN_IDLE1);					
					g_Player.ResetMaxSpeed();
					g_Player.pev.flags &= ~FL_LOCK_DUCK;
					g_Player.pev.flags &= ~FL_LOCK_JUMP;
				}
				break;
			}
		}
	}
	return CClientWeapon::ItemPostFrame();
}

void CClientMinigun::WeaponIdle(void)
{
	if(m_iShotsFired && UTIL_WeaponTimeBase() > m_flDecreaseShotsFired)
	{
		m_iShotsFired --;
		m_flDecreaseShotsFired = UTIL_WeaponTimeBase() + 0.1;
	}
}

float CClientMinigun::GetMaxSpeed(void)
{
	if (m_iSpin != STATE_MINIGUN_NONE)
		return 0.4935;

	return 1;
}