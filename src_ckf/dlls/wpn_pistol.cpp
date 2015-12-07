#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

enum pistol_e
{
	PISTOL_IDLE1,
	PISTOL_RELOAD,
	PISTOL_DRAW,
	PISTOL_FIRE
};

LINK_ENTITY_TO_CLASS(weapon_pistol, CPistol);

void CPistol::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_pistol");

	Precache();
	m_iId = WEAPON_PISTOL;
	m_bForEngineer = FALSE;
	SET_MODEL(ENT(pev), "models/CKF_III/wp_group_rf.mdl");
	pev->sequence = 5;
	m_iDefaultAmmo = PISTOL_DEFAULT_GIVE;

	FallInit();
}

void CPistol::Precache(void)
{
	PRECACHE_MODEL("models/CKF_III/v_pistol.mdl");
	PRECACHE_MODEL("models/CKF_III/v_pistol_engineer.mdl");
	PRECACHE_MODEL("models/CKF_III/wp_group_rf.mdl");
	PRECACHE_SOUND("CKF_III/pistol_shoot.wav");

	m_usFireScript = PRECACHE_EVENT(1, "events/ckf_pistol.sc");
}

int CPistol::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "SecondaryAmmo";
	p->iMaxAmmo1 =  _PISTOL_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = PISTOL_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 1;
	p->iId = m_iId = WEAPON_PISTOL;
	p->iFlags = 0;
	p->iWeight = PISTOL_WEIGHT;
	return 1;
}

int CPistol::iMaxAmmo1(void)
{
	return (!m_bForEngineer) ? _PISTOL_MAX_CARRY : _PISTOL_MAX_CARRY_ENG;
}

BOOL CPistol::Deploy(void)
{
	return GroupDeploy((!m_bForEngineer) ? "models/CKF_III/v_pistol.mdl" : "models/CKF_III/v_pistol_engineer.mdl", "models/CKF_III/wp_group_rf.mdl", PISTOL_DRAW, 0, 0, "onehanded", 15);
}

void CPistol::PrimaryAttack(void)
{
	PistolFire();
}

void CPistol::SecondaryAttack(void)
{
	m_pPlayer->Build_Undeploy();
}

void CPistol::PistolFire()
{
	float flSpread = 0.03;

	if(m_iShotsFired)
		flSpread *= min(1.0+m_iShotsFired/12.0f, 1.5);

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.2;
		}
		return;
	}

	int iCrit = m_pPlayer->GetCriticalFire(1, m_pPlayer->random_seed);

	m_iClip--;
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

	Vector vecAngles = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;

	UTIL_MakeVectors(vecAngles);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDst = m_pPlayer->CKFFireBullets(vecSrc, gpGlobals->v_forward, flSpread, 8192, BULLET_PLAYER_TF2, 15, iCrit, m_pPlayer->pev, m_pPlayer->random_seed, TRUE);

	//Draw Tracer
	int iTracerColor = (iCrit >= 2) ? m_pPlayer->m_iTeam : 0;//Draw

	ViewAnglesForPlayBack(vecAngles);
	PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFireScript, 0, vecSrc, vecAngles, flSpread, 0, iTracerColor, m_pPlayer->random_seed, (!m_iShotsFired) ? TRUE : FALSE, 0);

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.167f;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.9;

	m_flDecreaseShotsFired = m_flNextPrimaryAttack + 0.2f;

	if(m_iShotsFired < 5) m_iShotsFired ++;

	SendWeaponAnim(PISTOL_FIRE, UseDecrement() != FALSE);

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	m_fInReload = false;
}

void CPistol::Reload(void)
{
	if (DefaultReload(PISTOL_MAX_CLIP, PISTOL_RELOAD, (!m_bForEngineer) ? 1.25 : 1.36))
	{
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
	}
}

void CPistol::WeaponIdle(void)
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
		SendWeaponAnim(PISTOL_IDLE1, UseDecrement() != FALSE);
	}
}

void CPistol::PreAttachPlayer(CBasePlayer *pPlayer)
{
	m_bForEngineer = (pPlayer->m_iClass == CLASS_ENGINEER) ? TRUE : FALSE;
	if(m_bForEngineer)
	{
		m_iDefaultAmmo = m_iDefaultAmmo + _PISTOL_MAX_CARRY_ENG - _PISTOL_MAX_CARRY;
	}
}