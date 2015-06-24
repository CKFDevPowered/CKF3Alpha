#include <metahook.h>
#include "Renderer.h"
#include "configs.h"
#include "plugins.h"
#include "interface.h"

IRenderer *g_pRenderer = NULL;
ICKFClient *g_pCKFClient = NULL;

HINTERFACEMODULE g_hRenderer;
HINTERFACEMODULE g_hCKFClient;

cl_exportfuncs_t gRefClientFuncs;
cl_exportfuncs_t gCkfClientFuncs;

ref_export_t *gpRefExports = NULL;
ref_funcs_t *gpRefFuncs = NULL;
studio_funcs_t *gpStudioFuncs = NULL;

int BTE_GetHUDFov(void);
float *EV_GetPunchAngles(void);
pfnUserMsgHook MSG_HookUserMsg(char *szMsgName, pfnUserMsgHook pfn);
xcommand_t Cmd_HookCmd(char *cmd_name, xcommand_t newfuncs);

bte_funcs_t gBTEFuncs =
{
	BTE_GetHUDFov,
	EV_GetPunchAngles,
	MSG_HookUserMsg,
	Cmd_HookCmd
};

void Renderer_Init(void)
{
	if (!gConfigs.bEnableRenderer)
		return;

	char filepath[MAX_PATH];
	sprintf(filepath, "%s/renderer.dll", gConfigs.szGameDir);

	g_hRenderer = Sys_LoadModule(filepath);

	memset(&gRefClientFuncs, 0, sizeof(gRefClientFuncs));

	if (!g_hRenderer)
	{
		gConfigs.bEnableRenderer = false;
		return;
	}

	g_pRenderer = (IRenderer *)((CreateInterfaceFn)Sys_GetFactory(g_hRenderer))(RENDERER_API_VERSION, NULL);

	if (!g_pRenderer)
	{
		gConfigs.bEnableRenderer = false;
	}

	if(gConfigs.bEnableRenderer)
	{
		g_pRenderer->GetClientFuncs(&gRefClientFuncs);
		g_pRenderer->GetInterface(&gpRefExports, &gpRefFuncs, &gpStudioFuncs);
	}
}

void CKF_Init(void)
{
	char filepath[MAX_PATH];
	sprintf(filepath, "%s/ckf3.dll", gConfigs.szGameDir);

	g_hCKFClient = Sys_LoadModule(filepath);

	memset(&gCkfClientFuncs, 0, sizeof(gCkfClientFuncs));

	if (!g_hCKFClient)
		return;

	g_pCKFClient = (ICKFClient *)((CreateInterfaceFn)Sys_GetFactory(g_hCKFClient))(CKFCLIENT_API_VERSION, NULL);

	if (!g_hCKFClient)
		return;

	g_pCKFClient->GetClientFuncs(&gCkfClientFuncs);
	g_pCKFClient->GetBTEFuncs(&gBTEFuncs);
}

void Renderer_Shutdown(void)
{
	if (g_pRenderer)
		Sys_FreeModule(g_hRenderer);
}

void CKF_Shutdown(void)
{
	if (g_pCKFClient)
		Sys_FreeModule(g_hCKFClient);
}

void CKF_ShowScoreBoard(bool state)
{
	g_pCKFClient->ShowScoreBoard(state);
}

void CKF_SwitchWeapon(int slot)
{
	g_pCKFClient->SwitchWeapon(slot);
}

void CKF_Draw3DHUDStudioModel(cl_entity_t *pEntity, int x, int y, int w, int h)
{
	g_pCKFClient->Draw3DHUDStudioModel(pEntity, x, y, w, h);
}