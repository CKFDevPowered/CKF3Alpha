#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

enum smg_e
{
	SMG_IDLE1,
	SMG_RELOAD,
	SMG_DRAW,
	SMG_FIRE
};

LINK_ENTITY_TO_CLASS(weapon_smg, CSMG);

void CSMG::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_smg");

	Precache();
	m_iId = WEAPON_SMG;
	SET_MODEL(ENT(pev), "models/CKF_III/wp_group_rf.mdl");
	pev->sequence = 19;

	m_iDefaultAmmo = SMG_DEFAULT_GIVE;
	FallInit();
}

void CSMG::Precache(void)
{
	PRECACHE_MODEL("models/CKF_III/v_smg.mdl");
	PRECACHE_SOUND("CKF_III/smg_shoot.wav");
	m_usFireScript = PRECACHE_EVENT(1, "events/ckf_smg.sc");
}

int CSMG::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = "SecondaryAmmo";
	p->iMaxAmmo1 = _SMG_MAX_CARRY;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = SMG_MAX_CLIP;
	p->iSlot = 1;
	p->iPosition = 1;
	p->iId = m_iId = WEAPON_SMG;
	p->iFlags = 0;
	p->iWeight = SMG_WEIGHT;
	return 1;
}

BOOL CSMG::Deploy(void)
{
	return GroupDeploy("models/CKF_III/v_smg.mdl", "models/CKF_III/wp_group_rf.mdl", SMG_DRAW, 0, 0, "onehanded", 54);
	//return DefaultDeploy("models/CKF_III/v_smg.mdl", "models/CKF_III/wp_smg.mdl", SMG_DRAW, "onehanded", UseDecrement() != FALSE);
}

void CSMG::PrimaryAttack(void)
{
	SMGFire();
}

void CSMG::SMGFire(void)
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

	int iCrit = m_pPlayer->GetCriticalFire(1, m_pPlayer->random_seed);

	m_iClip--;
	m_pPlayer->pev->effects |= EF_MUZZLEFLASH;

	Vector vecAngles = m_pPlayer->pev->v_angle + m_pPlayer->pev->punchangle;
	UTIL_MakeVectors(vecAngles);
	Vector vecSrc = m_pPlayer->GetGunPosition();
	Vector vecDst = m_pPlayer->CKFFireBullets(vecSrc, gpGlobals->v_forward, flSpread, 8192, BULLET_PLAYER_TF2, 8, iCrit, m_pPlayer->pev, m_pPlayer->random_seed, TRUE);

	int iTracerColor = (iCrit >= 2) ? m_pPlayer->m_iTeam : 0;//Draw

	ViewAnglesForPlayBack(vecAngles);
	PLAYBACK_EVENT_FULL(FEV_NOTHOST, m_pPlayer->edict(), m_usFireScript, 0, vecSrc, vecAngles, flSpread, 0, iTracerColor, m_pPlayer->random_seed, (!m_iShotsFired) ? TRUE : FALSE, 0);

	m_pPlayer->m_iWeaponVolume = NORMAL_GUN_VOLUME;
	m_pPlayer->m_iWeaponFlash = NORMAL_GUN_FLASH;

	m_flNextPrimaryAttack = m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.1;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 1.9;

	m_flDecreaseShotsFired = m_flNextPrimaryAttack + 0.2;
	if(m_iShotsFired < 5) m_iShotsFired ++;

	SendWeaponAnim(SMG_FIRE, UseDecrement() != FALSE);

	m_pPlayer->SetAnimation(PLAYER_ATTACK1);
}

void CSMG::Reload(void)
{
	if (m_pPlayer->m_rgAmmo[m_iPrimaryAmmoType] <= 0)
		return;

	if (DefaultReload(SMG_MAX_CLIP, SMG_RELOAD, 1.4))
	{
		m_pPlayer->SetAnimation(PLAYER_RELOAD);
	}
}

void CSMG::WeaponIdle(void)
{
	ResetEmptySound();
	m_pPlayer->GetAutoaimVector(AUTOAIM_10DEGREES);

	if(m_iShotsFired && UTIL_WeaponTimeBase() > m_flDecreaseShotsFired)
	{
		m_iShotsFired --;
		m_flDecreaseShotsFired = UTIL_WeaponTimeBase() + 0.25;
	}

	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;
	SendWeaponAnim(SMG_IDLE1, UseDecrement() != FALSE);
}
