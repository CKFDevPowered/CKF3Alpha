#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

enum minigun_e
{
	MINIGUN_IDLE1,
	MINIGUN_SPIN_SHOOT,
	MINIGUN_DRAW,
	MINIGUN_WIND_UP,
	MINIGUN_WIND_DOWN,
	MINIGUN_SPIN_IDLE1
};

#define STATE_MINIGUN_NONE 0
#define STATE_MINIGUN_WINDUP 1
#define STATE_MINIGUN_SPIN 2
#define STATE_MINIGUN_FIRE 3
#define STATE_MINIGUN_FIRECRIT 4
#define STATE_MINIGUN_WINDDOWN 5

LINK_ENTITY_TO_CLASS(weapon_minigun, CMinigun);

void CMinigun::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_minigun");

	Precache();
	m_iId = WEAPON_MINIGUN;
	SET_MODEL(ENT(pev), "models/CKF_III/wp_group_2bone.mdl");
	pev->sequence = 1;
	m_iDefaultAmmo = MINIGUN_DEFAULT_GIVE;
	m_iSpin = STATE_MINIGUN_NONE;
	m_iPlayEmptySound = 2;
	FallInit();
}

void CMinigun::Precache(void)
{
	PRECACHE_MODEL("models/CKF_III/v_minigun.mdl");

	m_iShellId = PRECACHE_MODEL("models/rshell.mdl");

	PRECACHE_SOUND("CKF_III/minigun_shoot.wav");
	PRECACHE_SOUND("CKF_III/minigun_shoot_crit.wav");
	PRECACHE_SOUND("CKF_III/minigun_spin.wav");
	PRECACHE_SOUND("CKF_III/minigun_wind_up.wav");
	PRECACHE_SOUND("CKF_III/minigun_wind_down.wav");

	m_usFireScript = PRECACHE_EVENT(1, "events/ckf_minigun.sc");
}

int CMinigun::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "PrimaryAmmo";
	p->iMaxAmmo1 = _MINIGUN_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 1;
	p->iPosition = 0;
	p->iId = m_iId = WEAPON_MINIGUN;
	p->iFlags = 0;
	p->iWeight = MINIGUN_WEIGHT;

	return 1;
}

BOOL CMinigun::Deploy(void)
{
	m_iSpin = STATE_MINIGUN_NONE;
	
	return GroupDeploy("models/CKF_III/v_minigun.mdl", "models/CKF_III/wp_group_2bone.mdl", MINIGUN_DRAW, 0, 0, "shotgun", 3);
}

void CMinigun::Holster(int skiplocal)
{
	m_iSpin = STATE_MINIGUN_NONE;
	m_pPlayer->pev->flags &= ~FL_LOCK_DUCK;
	m_pPlayer->pev->flags &= ~FL_LOCK_JUMP;

	PLAYBACK_EVENT_FULL(FEV_GLOBAL | FEV_NOTHOST, m_pPlayer->edict(), m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, STATE_MINIGUN_NONE, 0, 0, 0);
	//EMIT_SOUND(ENT(m_pPlayer->pev) , CHAN_WEAPON, "CKF_III/null.wav", 1.0, 0.80); 
}

BOOL CMinigun::CanHolster(void)
{
	if(m_iSpin != STATE_MINIGUN_NONE)
		return FALSE;
	return TRUE;
}

void CMinigun::PrimaryAttack(void)
{
	if(m_iSpin < STATE_MINIGUN_SPIN || m_iSpin > STATE_MINIGUN_FIRECRIT)
		return;

	float flSpread = 0.10;
	if(m_iShotsFired)
		flSpread *= min(1.0+m_iShotsFired/20.0f, 1.5);

	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
		}
		return;
	}

	int iCrit = m_pPlayer->GetCriticalFire(1, m_pPlayer->random_seed);

	m_pPlayer->m_iWeaponVolume = LOUD_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = BRIGHT_GUN_FLASH;
	m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType]--;
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

	Vector vecAngles = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;
	UTIL_MakeVectors(vecAngles);
	Vector vecSrc = m_pPlayer->GetGunPosition();

	ClearMultiDamage();

	if(!m_iShotsFired) m_pPlayer->CKFFireBullets(vecSrc, gpGlobals->v_forward, 0.0, 8192, BULLET_PLAYER_TF2, 9, iCrit, m_pPlayer->pev, m_pPlayer->random_seed, FALSE);
	int iMaxBurst = (m_iShotsFired) ? 4 : 3;

	//Prepare to Draw Tracer
	int iTracerColor = (iCrit >= 2) ? m_pPlayer->m_iTeam : 0;//Draw

	for(int iBurst = 0; iBurst < iMaxBurst; iBurst ++)
	{
		Vector vecDst = m_pPlayer->CKFFireBullets(vecSrc, gpGlobals->v_forward, flSpread, 8192, BULLET_PLAYER_TF2, 9, iCrit, m_pPlayer->pev, m_pPlayer->random_seed+iBurst, FALSE);
	}

	ViewAnglesForPlayBack(vecAngles);

	PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFireScript, 0, vecSrc, vecAngles, flSpread, 0, iTracerColor, m_pPlayer->random_seed, (!m_iShotsFired) ? TRUE : FALSE, TRUE);

	ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

	//Fire event if necessary
	if(iCrit >= 2 && m_iSpin != STATE_MINIGUN_FIRECRIT)
	{
		m_iSpin = STATE_MINIGUN_FIRECRIT;
		SendWeaponAnim(MINIGUN_SPIN_SHOOT, UseDecrement() != FALSE);
		//EMIT_SOUND(ENT(m_pPlayer->pev) , CHAN_WEAPON, "CKF_III/minigun_shoot_crit.wav", 1.0, 0.80); 
		//PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, STATE_MINIGUN_FIRECRIT, 0, 0, 0);
	}
	else if(iCrit < 2 && m_iSpin != STATE_MINIGUN_FIRE)
	{
		m_iSpin = STATE_MINIGUN_FIRE;
		SendWeaponAnim(MINIGUN_SPIN_SHOOT, UseDecrement() != FALSE);
		//EMIT_SOUND(ENT(m_pPlayer->pev) , CHAN_WEAPON, "CKF_III/minigun_shoot.wav", 1.0, 0.80); 
		//PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, STATE_MINIGUN_FIRE, 0, 0, 0);
	}

	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.1;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.1;

	m_flDecreaseShotsFired = m_flNextPrimaryAttack + 0.2;

	if(m_iShotsFired < 10) m_iShotsFired ++;

	//m_pPlayer->m_flEjectBrass = gpGlobals->time + 0.05;

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
}

void CMinigun::ItemPostFrame(void)
{
	int iButton = m_pPlayer->pev->button;
	int iOldButton = m_pPlayer->pev->oldbuttons;

	if((m_iSpin == STATE_MINIGUN_FIRE || m_iSpin == STATE_MINIGUN_FIRECRIT) && !(iButton & IN_ATTACK))
	{
		m_iSpin = STATE_MINIGUN_SPIN;
		SendWeaponAnim(MINIGUN_SPIN_IDLE1, UseDecrement() != FALSE);
		//EMIT_SOUND(ENT(m_pPlayer->pev) , CHAN_WEAPON, "CKF_III/minigun_spin.wav", 1.0, 0.80);
		PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, STATE_MINIGUN_SPIN, 0, 0, 0);
	}
	else if((iButton & IN_ATTACK) || (iButton & IN_ATTACK2))
	{
		switch(m_iSpin)
		{
			case STATE_MINIGUN_NONE:
			{
				if(!(iButton & IN_DUCK) && !(iOldButton & IN_DUCK) && !m_pPlayer->pev->bInDuck)
				{
					m_iSpin = STATE_MINIGUN_WINDUP;
					m_fSpin = UTIL_WeaponTimeBase() + 0.87;
					SendWeaponAnim(MINIGUN_WIND_UP, UseDecrement() != FALSE);
					//EMIT_SOUND(ENT(m_pPlayer->pev) , CHAN_WEAPON, "CKF_III/minigun_wind_up.wav", 1.0, 0.80); 
					PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, STATE_MINIGUN_WINDUP, 0, 0, 0);

					m_pPlayer->pev->flags |= FL_LOCK_DUCK;
					m_pPlayer->pev->flags |= FL_LOCK_JUMP;
					//m_pPlayer->SetAnimation(PLAYER_RELOAD);
					m_pPlayer->ResetMaxSpeed();
				}
				break;
			}
			case STATE_MINIGUN_WINDUP:
			{
				if(m_fSpin < UTIL_WeaponTimeBase())
				{
					m_iSpin = STATE_MINIGUN_SPIN;
					SendWeaponAnim(MINIGUN_SPIN_IDLE1, UseDecrement() != FALSE);					
					//EMIT_SOUND(ENT(m_pPlayer->pev) , CHAN_WEAPON, "CKF_III/minigun_spin.wav", 1.0, 0.80);
					PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, STATE_MINIGUN_SPIN, 0, 0, 0);
					//m_pPlayer->SetAnimation(PLAYER_RELOAD);					
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
				SendWeaponAnim(MINIGUN_WIND_DOWN, UseDecrement() != FALSE);
				//EMIT_SOUND(ENT(m_pPlayer->pev) , CHAN_WEAPON, "CKF_III/minigun_wind_down.wav", 1.0, 0.80); 
				PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFireScript, 0, (float *)&g_vecZero, (float *)&g_vecZero, 0, 0, STATE_MINIGUN_WINDDOWN, 0, 0, 0);
				break;
			}
			case STATE_MINIGUN_WINDDOWN:
			{
				if(m_fSpin < UTIL_WeaponTimeBase())
				{
					m_iSpin = STATE_MINIGUN_NONE;
					SendWeaponAnim(MINIGUN_IDLE1, UseDecrement() != FALSE);					
					m_pPlayer->ResetMaxSpeed();
					m_pPlayer->pev->flags &= ~FL_LOCK_DUCK;
					m_pPlayer->pev->flags &= ~FL_LOCK_JUMP;
					//m_pPlayer->pev->button &= ~IN_ATTACK;
					//m_pPlayer->pev->button &= ~IN_ATTACK2;
					//m_pPlayer->pev->oldbuttons &= ~IN_ATTACK;
					//m_pPlayer->pev->oldbuttons &= ~IN_ATTACK2;
				}
				break;
			}
		}
	}
	return CBasePlayerWeapon::ItemPostFrame();
}

void CMinigun::WeaponIdle(void)
{
	if(m_iShotsFired && UTIL_WeaponTimeBase() > m_flDecreaseShotsFired)
	{
		m_iShotsFired --;
		m_flDecreaseShotsFired = UTIL_WeaponTimeBase() + 0.1;
	}
}

float CMinigun::GetMaxSpeed(void)
{
	if (m_iSpin != STATE_MINIGUN_NONE)
		return 0.4935;

	return 1;
}