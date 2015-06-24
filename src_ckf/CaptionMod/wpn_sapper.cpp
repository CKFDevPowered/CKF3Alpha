#include <metahook.h>
#include "plugins.h"
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "weapon.h"
#include "event.h"
#include "util.h"

void CClientSapper::Precache(void)
{
	
}

BOOL CClientSapper::Deploy(void)
{
	return GroupDeploy("models/CKF_III/v_sapper.mdl", "models/CKF_III/wp_group_rf.mdl", (g_Player.m_iCloak == CLOAK_NO) ? SAPPER_DRAW : SAPPER_DRAW2, 0, 0, "onehanded");
}

void CClientSapper::ItemPostFrame(void)
{
	CClientWeapon::ItemPostFrame();

	if(*cls_viewmodel_sequence == SAPPER_DRAW && *cls_viewmodel_starttime != 0 && g_flClientTime - *cls_viewmodel_starttime > 30.0f/30.0f)
	{
		SendWeaponAnim(SAPPER_IDLE1);
	}
}