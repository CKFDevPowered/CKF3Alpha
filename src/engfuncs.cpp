#include <metahook.h>
#include <com_model.h>
#include "configs.h"
#include "ViewPort.h"
#include "plugins.h"

cl_enginefunc_t *g_pEngfuncs = NULL;

void GetEngfuncs(void)
{
	g_pEngfuncs = g_pMetaSave->pEngineFuncs;
}

struct model_s *LoadMapSprite(const char *filename)
{
	return gEngfuncs.LoadMapSprite(filename);
}

int GetScreenInfo(SCREENINFO *pscrinfo)
{
	return gEngfuncs.pfnGetScreenInfo(pscrinfo);
}

void HookEvent(char *name, void (*pfnEvent)(struct event_args_s *args))
{
}

int CheckParm(char *parm, char **ppnext)
{
/*	if (ppnext == NULL && (!strcmp(parm, "-nomouse") || !strcmp(parm, "-nojoy")))
		return 1;
*/
	return gEngfuncs.CheckParm(parm, ppnext);
}

void GetPlayerInfo(int ent_num, hud_player_info_t *pinfo)
{
	static hud_player_info_t info;
	g_pMetaSave->pEngineFuncs->pfnGetPlayerInfo(ent_num, &info);

	pinfo->name = info.name;
	pinfo->ping = info.ping;
	pinfo->thisplayer = info.thisplayer;
	pinfo->spectator = info.spectator;
	pinfo->packetloss = info.packetloss;
	pinfo->model = info.model;
	pinfo->topcolor = info.topcolor;
	pinfo->bottomcolor = info.bottomcolor;

	if (g_dwEngineBuildnum >= 5953)
		pinfo->m_nSteamID = info.m_nSteamID;
}

void SetCrosshair(HSPRITE hspr, wrect_t rc, int r, int g, int b)
{
}