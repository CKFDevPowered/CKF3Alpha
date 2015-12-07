#include <metahook.h>
#include "exportfuncs.h"
#include "engfuncs.h"
#include "ref_int.h"

#include <IPlugins.h>
#include <ICKFClient.h>
#include <IRenderer.h>
#include <IBTEClient.h>

cl_exportfuncs_t gExportfuncs;
mh_interface_t *g_pInterface;
metahook_api_t *g_pMetaHookAPI;
mh_enginesave_t *g_pMetaSave;
IFileSystem *g_pFileSystem;

DWORD g_dwEngineBase, g_dwEngineSize;
DWORD g_dwClientBase, g_dwClientSize;
DWORD g_dwEngineBuildnum;
DWORD g_iVideoMode;
int g_iVideoWidth, g_iVideoHeight, g_iBPP;
bool g_bWindowed;

extern ckf_vars_t gCKFVars;

void Engine_InstallHook(void);
void Client_InstallHook(void);

//Renderer.dll
void Renderer_Init(void);

//CSBTE.dll
void BTE_Init(void);
HINTERFACEMODULE g_hBTEClient;
IBTEClient *g_pBTEClient = NULL;

void IPlugins::Init(metahook_api_t *pAPI, mh_interface_t *pInterface, mh_enginesave_t *pSave)
{
	g_pInterface = pInterface;
	g_pMetaHookAPI = pAPI;
	g_pMetaSave = pSave;

	g_pInterface->CommandLine->RemoveParm("-d3d");
	g_pInterface->CommandLine->AppendParm("-gl", NULL);
	g_pInterface->CommandLine->AppendParm("-32bpp", NULL);
	//g_pInterface->CommandLine->AppendParm("-nocdaudio", NULL);
	//g_pInterface->CommandLine->AppendParm("-nomaster", NULL);
	//g_pInterface->CommandLine->AppendParm("-insecure", NULL);
	//g_pInterface->CommandLine->AppendParm("-forcevalve", NULL);
}

void IPlugins::Shutdown(void)
{
}

void IPlugins::LoadEngine(void)
{
	g_pFileSystem = g_pInterface->FileSystem;
	g_iVideoMode = g_pMetaHookAPI->GetVideoMode(&g_iVideoWidth, &g_iVideoHeight, &g_iBPP, &g_bWindowed);
	g_dwEngineBuildnum = g_pMetaHookAPI->GetEngineBuildnum();

	if (g_iVideoMode == VIDEOMODE_SOFTWARE)
		return;

	g_dwEngineBase = g_pMetaHookAPI->GetEngineBase();
	g_dwEngineSize = g_pMetaHookAPI->GetEngineSize();

	BTE_Init();
	Renderer_Init();
	Engine_InstallHook();
}

void IPlugins::LoadClient(cl_exportfuncs_t *pExportFunc)
{
	HMODULE hClient = GetModuleHandle("client.dll");
	if(!hClient)
	{
		g_dwClientBase = 0x1901000;
		g_dwClientSize = 0xBC000;
	}
	else
	{
		g_dwClientBase = g_pMetaHookAPI->GetModuleBase(hClient);
		g_dwClientSize = g_pMetaHookAPI->GetModuleSize(hClient);
	}
	memcpy(&gExportfuncs, pExportFunc, sizeof(gExportfuncs));

	Client_InstallHook();
}

void IPlugins::ExitGame(int iResult)
{
}

EXPOSE_SINGLE_INTERFACE(IPlugins, IPlugins, METAHOOK_PLUGIN_API_VERSION);

//CKF Exports

extern cl_exportfuncs_t gClientfuncs;

void ICKFClient::GetClientFuncs(cl_exportfuncs_t *pExportFuncs)
{
	memcpy(pExportFuncs, &gClientfuncs, sizeof(cl_exportfuncs_t));
}

int HUD_SwitchWeapon(int slot);

int ICKFClient::SwitchWeapon(int slot)
{
	return HUD_SwitchWeapon(slot);
}

void R_Draw3DHUDStudioModel(cl_entity_t *pEntity, int x, int y, int w, int h, qboolean bLocalXY);

void ICKFClient::Draw3DHUDStudioModel(cl_entity_t *pEntity, int x, int y, int w, int h, qboolean bLocalXY)
{
	R_Draw3DHUDStudioModel(pEntity, x, y, w, h, bLocalXY);
}

void ICKFClient::GetCKFVars(ckf_vars_t *pCKFVars)
{
	memcpy(pCKFVars, &gCKFVars, sizeof(ckf_vars_t));
}

void DrawHudMask(int col, int x, int y, int w, int h);

void ICKFClient::DrawHudMask(int col, int x, int y, int w, int h)
{
	::DrawHudMask(col, x, y, w, h);
}

void BaseUI_Initalize(CreateInterfaceFn *factories, int count);

void ICKFClient::BaseUI_Initalize(CreateInterfaceFn *factories, int count)
{
	::BaseUI_Initalize(factories, count);
}

EXPOSE_SINGLE_INTERFACE(ICKFClient, ICKFClient, CKFCLIENT_API_VERSION);