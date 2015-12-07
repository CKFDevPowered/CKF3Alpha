#include <metahook.h>
#include <r_studioint.h>
#include <r_efx.h>
#include <ref_params.h>
#include <cl_entity.h>
#include <studio.h>
#include <cvardef.h>
#include <usercmd.h>
#include "engfuncs.h"
#include "qgl.h"
#include "hud.h"
#include "svchook.h"
#include "msghook.h"
#include "cmd.h"
#include "BaseUI.h"
#include "mempatchs.h"
#include "common.h"
#include "configs.h"
#include "events.h"
#include "client.h"
#include "player.h"
#include "weapons.h"
#include "eventscripts.h"
#include <pm_defs.h>
#include <event_api.h>
#include "event.h"
#include "Video.h"
#include "Renderer.h"
#include "plugins.h"
#include "modules.h"
#include "inputw32.h"
#include "entity_types.h"
#include "game_shared/voice_status.h"
#include "developer.h"
#include "zone.h"
#include "cvar.h"
#include "vgui_int.h"

#include "ref_int.h"

extern "C"
{
#include "pm_shared.h"
}

cl_enginefunc_t gEngfuncs;

void InitInput(void);
void ShutdownInput(void);

int Initialize(struct cl_enginefuncs_s *pEnginefuncs, int iVersion)
{
	developer = Cvar_FindVar("developer");

	engine = pEnginefuncs;
	memcpy(&gEngfuncs, pEnginefuncs, sizeof(gEngfuncs));

	//pEnginefuncs->pfnHookEvent = HookEvent;
	pEnginefuncs->CheckParm = CheckParm;
	pEnginefuncs->pfnGetPlayerInfo = GetPlayerInfo;
	pEnginefuncs->pfnGetScreenInfo = GetScreenInfo;
	pEnginefuncs->pfnSetCrosshair = SetCrosshair;

	// 应该换个位置
	//{
		Cache_Init();
		COM_Init();
		VID_Init();
		//QGL_Init is called at LoadEngine for drawing loading screen
		QGL_Init();
		SVC_Init();
		MSG_Init();

		VGui_Startup();
	//}

	int result = gExportfuncs.Initialize(pEnginefuncs, iVersion);

	if (result)
	{
		EV_InstallHook();
		IN_InstallHook();
	}

	if(gCkfClientFuncs.Initialize)
		gCkfClientFuncs.Initialize(pEnginefuncs, iVersion);

	return result;
}

void *ClientFactory(void)
{
	if (gConfigs.bEnableClientUI)
		return Sys_GetFactoryThis();

	return gExportfuncs.ClientFactory();
}

void HUD_Init(void)
{
	gExportfuncs.HUD_Init();

	InitInput();

	CL_Init();
	EV_Init();
	gHUD.Init();

	if(gCkfClientFuncs.HUD_Init)
		gCkfClientFuncs.HUD_Init();
}

int HUD_VidInit(void)
{
	int result = gExportfuncs.HUD_VidInit();

	CL_VidInit();
	gHUD.VidInit();

	if(gCkfClientFuncs.HUD_VidInit)
		gCkfClientFuncs.HUD_VidInit();
	return result;
}

int HUD_Redraw(float time, int intermission)
{
	DWORD dwAddress = (DWORD)g_pMetaHookAPI->SearchPattern(g_pMetaSave->pExportFuncs->HUD_VidInit, 0x10, "\xB9", 1);

	if (dwAddress)
	{
		static DWORD pHUD = *(DWORD *)(dwAddress + 0x1);
		HUDLIST *pHudList = (HUDLIST *)(*(DWORD *)(pHUD + 0x0));

		while (pHudList)
		{
			pHudList->p->m_iFlags = 0;
			pHudList = pHudList->pNext;
		}
	}

	gHUD.Redraw(time, intermission);

	if(gCkfClientFuncs.HUD_Redraw)
		gCkfClientFuncs.HUD_Redraw(time, intermission);
	return 1;
}

int HUD_UpdateClientData(client_data_t *pcldata, float flTime)
{
	IN_Commands();

	gHUD.UpdateClientData(pcldata, flTime);

	if(gCkfClientFuncs.HUD_UpdateClientData)
		gCkfClientFuncs.HUD_UpdateClientData(pcldata, flTime);

	return gExportfuncs.HUD_UpdateClientData(pcldata, flTime);
}

char HUD_PlayerMoveTexture(char *name)
{
	if(gCkfClientFuncs.HUD_PlayerMoveTexture)
		return gCkfClientFuncs.HUD_PlayerMoveTexture(name);
	return gExportfuncs.HUD_PlayerMoveTexture(name);
}

void HUD_PlayerMove(struct playermove_s *ppmove, int server)
{
	gEngfuncs.pEventAPI->EV_LocalPlayerViewheight(g_vecEyePos);

	g_vecEyePos[0] += ppmove->origin[0];
	g_vecEyePos[1] += ppmove->origin[1];
	g_vecEyePos[2] += ppmove->origin[2];

	if(gCkfClientFuncs.HUD_PlayerMove)
		return gCkfClientFuncs.HUD_PlayerMove(ppmove, server);

	return gExportfuncs.HUD_PlayerMove(ppmove, server);
}

void HUD_PlayerMoveInit(struct playermove_s *ppmove)
{
	gExportfuncs.HUD_PlayerMoveInit(ppmove);

	//init pmove so that PM_GetPhysEntInfo don't crash
	PM_Init(ppmove);

	if(gCkfClientFuncs.HUD_PlayerMoveInit)
		gCkfClientFuncs.HUD_PlayerMoveInit(ppmove);
}

void HUD_Frame(double time)
{
	VID_Frame();
	CL_Frame();

	GetClientVoice()->Frame(time);

	if(gCkfClientFuncs.HUD_Frame)
		gCkfClientFuncs.HUD_Frame(time);
}

void HUD_Shutdown(void)
{
	ShutdownInput();

	CL_Shutdown();
	EVS_ReleaseAllScript();

	if(gCkfClientFuncs.HUD_Shutdown)
		gCkfClientFuncs.HUD_Shutdown();

	return gExportfuncs.HUD_Shutdown();
}

void HUD_TempEntUpdate(double frametime, double client_time, double cl_gravity, struct tempent_s **ppTempEntFree, struct tempent_s **ppTempEntActive, int (*Callback_AddVisibleEntity)(struct cl_entity_s *pEntity), void (*Callback_TempEntPlaySound)(struct tempent_s *pTemp, float damp))
{
	if(gCkfClientFuncs.HUD_TempEntUpdate)
		gCkfClientFuncs.HUD_TempEntUpdate(frametime, client_time, cl_gravity, ppTempEntFree, ppTempEntActive, Callback_AddVisibleEntity, Callback_TempEntPlaySound);

	gExportfuncs.HUD_TempEntUpdate(frametime, client_time, cl_gravity, ppTempEntFree, ppTempEntActive, Callback_AddVisibleEntity, Callback_TempEntPlaySound);
}

int HUD_ConnectionlessPacket(const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size)
{
	if(gCkfClientFuncs.HUD_ConnectionlessPacket)
		gCkfClientFuncs.HUD_ConnectionlessPacket(net_from, args, response_buffer, response_buffer_size);

	return CL_ConnectionlessPacket(net_from, args, response_buffer, response_buffer_size);
}