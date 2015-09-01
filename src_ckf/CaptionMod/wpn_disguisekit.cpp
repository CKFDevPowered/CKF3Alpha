#include <metahook.h>
#include "plugins.h"
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "weapon.h"
#include "event.h"

void CClientDisguiseKit::Precache(void)
{
	
}

BOOL CClientDisguiseKit::Deploy(void)
{
	//if(g_runfuncs)
	//{
	//ShowHudMenu(HUDMENU_DISGUISE, 0);
	//}

	return GroupDeploy("models/CKF_III/v_disguisekit.mdl", "models/CKF_III/wp_group_rf.mdl", DISGUISEKIT_DRAW, 0, 0, "c4");
}

void CClientDisguiseKit::Holster(void)
{
	//if(g_runfuncs)
	//{
	//	ShowHudMenu(MENU_CLOSE, 0);
	//}
}

void CClientDisguiseKit::WeaponIdle(void)
{
	if (m_flTimeWeaponIdle > UTIL_WeaponTimeBase())
		return;

	m_flTimeWeaponIdle = UTIL_WeaponTimeBase() + 20;
	SendWeaponAnim(DISGUISEKIT_IDLE1);
}