#include <metahook.h>
#include "plugins.h"
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "weapon.h"
#include "event.h"

void CClientDestroyPDA::Precache(void)
{
	
}

BOOL CClientDestroyPDA::Deploy(void)
{
	//if(g_runfuncs)
	//{
	//	ShowHudMenu(HUDMENU_DEMOLISH, 0);
	//}

	return GroupDeploy("models/CKF_III/v_destroypda.mdl", "models/CKF_III/wp_group_rf.mdl", DESTROYPDA_DRAW, 0, 0, "c4");
}

void CClientDestroyPDA::Holster(void)
{
	//if(g_runfuncs)
	//{
	//	ShowHudMenu(MENU_CLOSE, 0);
	//}
}

BOOL CClientDestroyPDA::CanHolster(void)
{
	if(g_iHudMenu == HUDMENU_BUILD || g_Player.m_iCarryBluePrint)
		return FALSE;

	return CClientWeapon::CanDeploy();
}

void CClientDestroyPDA::WeaponIdle(void)
{
	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;
	SendWeaponAnim(DESTROYPDA_IDLE1);
}