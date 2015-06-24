#include <metahook.h>
#include "plugins.h"
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "weapon.h"
#include "event.h"

void CL_BluePrint(int bp);

void CClientBuildPDA::Precache(void)
{
	
}

void CClientBuildPDA::SecondaryAttack(void)
{
	if(!g_Player.m_iCarryBluePrint)
		return;

	if(m_bDelayedFire)
		return;

	if(g_runfuncs)
	{
		CL_BluePrint(10 + g_Player.m_iBluePrintYaw);
	}

	g_Player.m_iBluePrintYaw ++;
	if(g_Player.m_iBluePrintYaw > 3)
		g_Player.m_iBluePrintYaw = 0;

	m_bDelayedFire = TRUE;
	m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.8;
	m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.8;
}

BOOL CClientBuildPDA::Deploy(void)
{
	//if(g_runfuncs)
	//{
	//	ShowHudMenu(HUDMENU_BUILD, 0);
	//}

	return GroupDeploy("models/CKF_III/v_buildpda.mdl", "models/CKF_III/wp_group_rf.mdl", BUILDPDA_DRAW, 0, 0, "c4");
}

void CClientBuildPDA::Holster(void)
{
	//if(g_runfuncs)
	//{
	//	ShowHudMenu(MENU_CLOSE, 0);
	//}

	g_Player.m_iCarryBluePrint = 0;
}

BOOL CClientBuildPDA::CanHolster(void)
{
	if(g_iHudMenu == HUDMENU_DEMOLISH || g_Player.m_iCarryBluePrint)
		return FALSE;

	return TRUE;
}

void CClientBuildPDA::WeaponIdle(void)
{
	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;
	SendWeaponAnim(BUILDPDA_IDLE1);
}