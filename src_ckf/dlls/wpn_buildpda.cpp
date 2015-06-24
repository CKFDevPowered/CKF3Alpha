#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

enum buildpda_e
{
	BUILDPDA_IDLE1,
	BUILDPDA_DRAW
};

LINK_ENTITY_TO_CLASS(weapon_buildpda, CBuildPDA);

void CBuildPDA::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_buildpda");

	Precache();
	m_iId = WEAPON_BUILDPDA;
	SET_MODEL(ENT(pev), "models/CKF_III/wp_group_rf.mdl");
	pev->sequence = 13;
	m_iDefaultAmmo = -1;
	FallInit();
}

void CBuildPDA::Precache(void)
{
	PRECACHE_MODEL("models/CKF_III/v_buildpda.mdl");
}

int CBuildPDA::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 3;
	p->iPosition = 1;
	p->iId = m_iId = WEAPON_BUILDPDA;
	p->iFlags = 0;
	p->iWeight = BUILDPDA_WEIGHT;
	return 1;
}

void CBuildPDA::PrimaryAttack(void)
{
	if(!m_pPlayer->m_iCarryBluePrint)
		return;

	m_pPlayer->Build_Deploy();
}

void CBuildPDA::SecondaryAttack(void)
{
	if(!m_pPlayer->m_iCarryBluePrint)
		return;

	if(m_bDelayedFire)
		return;

	m_pPlayer->m_iBluePrintYaw ++;
	if(m_pPlayer->m_iBluePrintYaw > 3)
		m_pPlayer->m_iBluePrintYaw = 0;

	m_bDelayedFire = TRUE;
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.8;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.8;
}

BOOL CBuildPDA::Deploy(void)
{
	ShowHudMenu(m_pPlayer, HUDMENU_BUILD, 0, FALSE);

	return GroupDeploy("models/CKF_III/v_buildpda.mdl", "models/CKF_III/wp_group_rf.mdl", BUILDPDA_DRAW, 0, 0, "c4", 39);
}

void CBuildPDA::Holster(int skiplocal)
{
	ShowHudMenu(m_pPlayer, MENU_CLOSE, 0, FALSE);

	m_pPlayer->m_iCarryBluePrint = 0;
}

BOOL CBuildPDA::CanDeploy(void)
{
	if(m_pPlayer->m_iHudMenu == HUDMENU_DEMOLISH || m_pPlayer->m_iCarryBluePrint)
		return FALSE;

	return TRUE;
}

void CBuildPDA::WeaponIdle(void)
{
	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;
	SendWeaponAnim(BUILDPDA_IDLE1, UseDecrement() != FALSE);
}