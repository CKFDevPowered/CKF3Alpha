#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

enum revolver_e
{
	REVOLVER_IDLE1,
	REVOLVER_RELOAD,
	REVOLVER_DRAW,
	REVOLVER_FIRE
};

LINK_ENTITY_TO_CLASS(weapon_revolver, CRevolver);

void CRevolver::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_revolver");

	Precache();
	m_iId = WEAPON_REVOLVER;
	SET_MODEL(ENT(pev), "models/CKF_III/wp_group_rf.mdl");
	pev->sequence = 16;

	m_iDefaultAmmo = REVOLVER_DEFAULT_GIVE;
	FallInit();
}

void CRevolver::Precache(void)
{
	PRECACHE_MODEL("models/CKF_III/v_revolver.mdl");
	PRECACHE_SOUND("CKF_III/revolver_shoot.wav");
	m_usFireScript = PRECACHE_EVENT(1, "events/ckf_revolver.sc");
}

int CRevolver::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "PrimaryAmmo";
	p->iMaxAmmo1 = _REVOLVER_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = REVOLVER_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 1;
	p->iId = m_iId = WEAPON_REVOLVER;
	p->iFlags = 0;
	p->iWeight = REVOLVER_WEIGHT;
	return 1;
}

BOOL CRevolver::Deploy(void)
{
	return GroupDeploy("models/CKF_III/v_revolver.mdl", "models/CKF_III/wp_group_rf.mdl", REVOLVER_DRAW, 0, 0, "shotgun", 48);
}

void CRevolver::PrimaryAttack(void)
{
	RevolverFire();
}

void CRevolver::RevolverFire(void)
{
	float flSpread = 0.02;
	if(m_iShotsFired) flSpread *= min(1.0+m_iShotsFired/5.0f, 1.3);

	if (m_iClip <= 0)
	{
		if (m_fFireOnEmpty)
		{
			PlayEmptySound();
			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.58;
		}
		return;
	}

	int iCrit = m_pPlayer->GetCriticalFire(0, m_pPlayer->random_seed);

	m_iClip--;
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

	Vector vecAngles = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;
	UTIL_MakeVectors(vecAngles);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDst = m_pPlayer->CKFFireBullets(vecSrc, gpGlobals->v_forward, flSpread, 8192, BULLET_PLAYER_TF2, 40, iCrit, m_pPlayer->pev, m_pPlayer->random_seed, TRUE);

	//Draw Tracer
	int iTracerColor = (iCrit >= 2) ? m_pPlayer->m_iTeam : 0;

	ViewAnglesForPlayBack(vecAngles);
	PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFireScript, 0, vecSrc, vecAngles, flSpread, 0, iTracerColor, m_pPlayer->random_seed, (!m_iShotsFired) ? TRUE : FALSE, 0);

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;
	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.58f;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.9;

	m_flDecreaseShotsFired = m_flNextPrimaryAttack + 0.4167f;
	if(m_iShotsFired < 3) m_iShotsFired ++;

	m_pPlayer->Disguise_Stop();

	SendWeaponAnim(REVOLVER_FIRE, UseDecrement() != FALSE);

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);

	m_fInReload = false;
}

void CRevolver::SecondaryAttack(void)
{
	if(m_pPlayer->m_iCloak == CLOAK_NO)
		m_pPlayer->Cloak_Begin();
	else if(m_pPlayer->m_iCloak == CLOAK_YES)
		m_pPlayer->Cloak_Stop();
}

void CRevolver::Reload(void)
{
	if (DefaultReload(REVOLVER_MAX_CLIP, REVOLVER_RELOAD, 1.16))
	{
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
	}
}

void CRevolver::WeaponIdle(void)
{
	ResetEmptySound();

	if(m_iShotsFired && UTIL_WeaponTimeBase() > m_flDecreaseShotsFired)
	{
		m_iShotsFired --;
		m_flDecreaseShotsFired = UTIL_WeaponTimeBase() + 0.4167f;
	}

	if (m_flTimeWeaponIdle < UTIL_WeaponTimeBase())
	{
		m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;
		SendWeaponAnim(REVOLVER_IDLE1, UseDecrement() != FALSE);
	}
}