#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

enum destroypda_e
{
	DESTROYPDA_IDLE1,
	DESTROYPDA_DRAW
};

LINK_ENTITY_TO_CLASS(weapon_destroypda, CDestroyPDA);

void CDestroyPDA::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_destroypda");

	Precache();
	m_iId = WEAPON_DESTROYPDA;
	SET_MODEL(ENT(pev), "models/CKF_III/wp_group_rf.mdl");
	pev->sequence = 13;
	m_iDefaultAmmo = -1;
	FallInit();
}

void CDestroyPDA::Precache(void)
{
	PRECACHE_MODEL("models/CKF_III/v_destroypda.mdl");
}

int CDestroyPDA::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 4;
	p->iPosition = 1;
	p->iId = m_iId = WEAPON_DESTROYPDA;
	p->iFlags = 0;
	p->iWeight = DESTROYPDA_WEIGHT;
	return 1;
}

BOOL CDestroyPDA::Deploy(void)
{
	ShowHudMenu(m_pPlayer, HUDMENU_DEMOLISH, 0, FALSE);

	return GroupDeploy("models/CKF_III/v_destroypda.mdl", "models/CKF_III/wp_group_rf.mdl", DESTROYPDA_DRAW, 0, 0, "c4", 39);
}

void CDestroyPDA::Holster(int skiplocal)
{
	ShowHudMenu(m_pPlayer, MENU_CLOSE, 0, FALSE);
}

BOOL CDestroyPDA::CanDeploy(void)
{
	if(m_pPlayer->m_iHudMenu == HUDMENU_BUILD || m_pPlayer->m_iCarryBluePrint)
		return FALSE;

	return CBasePlayerWeapon::CanDeploy();
}

void CDestroyPDA::WeaponIdle(void)
{
	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;
	SendWeaponAnim(DESTROYPDA_IDLE1, UseDecrement() != FALSE);
}