#include <metahook.h>
#include "gl_local.h"
#include "exportfuncs.h"
#include <IRenderer.h>
#include <IBTEClient.h>

cl_exportfuncs_t gExportfuncs;
mh_interface_t *g_pInterface;
metahook_api_t *g_pMetaHookAPI;
mh_enginesave_t *g_pMetaSave;
IFileSystem *g_pFileSystem;

HINSTANCE g_hInstance, g_hThisModule, g_hEngineModule;
DWORD g_dwEngineBase, g_dwEngineSize;
DWORD g_dwEngineBuildnum;
DWORD g_iVideoMode;
int g_iVideoWidth, g_iVideoHeight, g_iBPP;

#pragma pack(1)
bool g_bWindowed;
bool g_bIsNewEngine;
bool g_bIsUseSteam;
bool g_bIsDebuggerPresent;
#pragma pack()

void IPlugins::Init(metahook_api_t *pAPI, mh_interface_t *pInterface, mh_enginesave_t *pSave)
{
	BOOL (*IsDebuggerPresent)(void) = (BOOL (*)(void))GetProcAddress(GetModuleHandle("kernel32.dll"), "IsDebuggerPresent");

	g_pInterface = pInterface;
	g_pMetaHookAPI = pAPI;
	g_pMetaSave = pSave;
	g_hInstance = GetModuleHandle(NULL);
	g_bIsDebuggerPresent = IsDebuggerPresent() != FALSE;

	g_pInterface->CommandLine->RemoveParm("-d3d");
	g_pInterface->CommandLine->AppendParm("-gl", NULL);
	g_pInterface->CommandLine->AppendParm("-32bpp", NULL);
}

void IPlugins::Shutdown(void)
{
}

void IPlugins::LoadEngine(void)
{
	g_pFileSystem = g_pInterface->FileSystem;
	g_iVideoMode = g_pMetaHookAPI->GetVideoMode(&g_iVideoWidth, &g_iVideoHeight, &g_iBPP, &g_bWindowed);
	g_dwEngineBuildnum = g_pMetaHookAPI->GetEngineBuildnum();
	g_iBPP = 32;

	g_hEngineModule = g_pMetaHookAPI->GetEngineModule();
	g_dwEngineBase = g_pMetaHookAPI->GetEngineBase();
	g_dwEngineSize = g_pMetaHookAPI->GetEngineSize();

	BTE_Init();
	R_FillAddress();
	R_InstallHook();
}

void IPlugins::LoadClient(cl_exportfuncs_t *pExportFunc)
{
	memcpy(&gExportfuncs, pExportFunc, sizeof(gExportfuncs));
}

void IPlugins::ExitGame(int iResult)
{
}

EXPOSE_SINGLE_INTERFACE(IPlugins, IPlugins, METAHOOK_PLUGIN_API_VERSION);

//renderer exports

extern cl_exportfuncs_t gClientfuncs;

void IRenderer::GetClientFuncs(cl_exportfuncs_t *pExportFuncs)
{
	memcpy(pExportFuncs, &gClientfuncs, sizeof(cl_exportfuncs_t));
}

void IRenderer::GetInterface(ref_export_t *pRefExports, const char *version)
{
	if(!strcmp(version, META_RENDERER_VERSION))
	{
		memcpy(pRefExports, &gRefExports, sizeof(ref_export_t));
	}
	else
	{
		g_pMetaSave->pEngineFuncs->Con_Printf("Meta Renderer interface version (%s) should be (%s)\n", version, META_RENDERER_VERSION);
	}
}

EXPOSE_SINGLE_INTERFACE(IRenderer, IRenderer, RENDERER_API_VERSION);