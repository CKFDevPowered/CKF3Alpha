#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "player.h"
#include "weapons.h"

enum disguisekit_e
{
	DISGUISEKIT_IDLE1,
	DISGUISEKIT_DRAW
};

LINK_ENTITY_TO_CLASS(weapon_disguisekit, CDisguiseKit);

void CDisguiseKit::Spawn(void)
{
	pev->classname = MAKE_STRING("weapon_disguisekit");

	Precache();
	m_iId = WEAPON_DISGUISEKIT;
	SET_MODEL(ENT(pev), "models/CKF_III/wp_group_rf.mdl");
	pev->sequence = 15;
	m_iDefaultAmmo = -1;
	FallInit();
}

void CDisguiseKit::Precache(void)
{
	PRECACHE_MODEL("models/CKF_III/v_disguisekit.mdl");
}

int CDisguiseKit::GetItemInfo(ItemInfo *p)
{
	p->pszName = STRING(pev->classname);
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 3;
	p->iPosition = 1;
	p->iId = m_iId = WEAPON_DISGUISEKIT;
	p->iFlags = 0;
	p->iWeight = DISGUISEKIT_WEIGHT;
	return 1;
}

BOOL CDisguiseKit::Deploy(void)
{
	ShowHudMenu(m_pPlayer, HUDMENU_DISGUISE, 0, FALSE);

	return GroupDeploy("models/CKF_III/v_disguisekit.mdl", "models/CKF_III/wp_group_rf.mdl", DISGUISEKIT_DRAW, 0, 0, "onehanded", 45);
}

void CDisguiseKit::Holster(int skiplocal)
{
	ShowHudMenu(m_pPlayer, MENU_CLOSE, 0, FALSE);
}

void CDisguiseKit::SecondaryAttack(void)
{
	if(m_pPlayer->m_iCloak == CLOAK_NO)
		m_pPlayer->Cloak_Begin();
	else if(m_pPlayer->m_iCloak == CLOAK_YES)
		m_pPlayer->Cloak_Stop();
}

void CDisguiseKit::WeaponIdle(void)
{
	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;
	SendWeaponAnim(DISGUISEKIT_IDLE1, UseDecrement() != FALSE);
}