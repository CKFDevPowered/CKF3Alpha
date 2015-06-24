#include <metahook.h>
#include "gl_local.h"
#include "exportfuncs.h"
#include <IRenderer.h>

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

void IRenderer::Init(metahook_api_t *pAPI, mh_interface_t *pInterface, mh_enginesave_t *pSave)
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

void IRenderer::Shutdown(void)
{
}

void IRenderer::LoadEngine(void)
{
	g_pFileSystem = g_pInterface->FileSystem;
	g_iVideoMode = g_pMetaHookAPI->GetVideoMode(&g_iVideoWidth, &g_iVideoHeight, &g_iBPP, &g_bWindowed);
	g_dwEngineBuildnum = g_pMetaHookAPI->GetEngineBuildnum();
	g_iBPP = 32;

	if (g_iVideoMode == VIDEOMODE_SOFTWARE)
		return;

	g_hEngineModule = g_pMetaHookAPI->GetEngineModule();
	g_dwEngineBase = g_pMetaHookAPI->GetEngineBase();
	g_dwEngineSize = g_pMetaHookAPI->GetEngineSize();
	g_bIsNewEngine = false;

	if (g_iVideoMode != VIDEOMODE_OPENGL)
		return;

	API_InstallHook();
	R_FillAddress();
	R_InstallHook();
}

void IRenderer::LoadClient(cl_exportfuncs_t *pExportFunc)
{
	if (g_iVideoMode != VIDEOMODE_OPENGL)
		return;

	memcpy(&gExportfuncs, pExportFunc, sizeof(gExportfuncs));
}

void IRenderer::ExitGame(int iResult)
{
}

extern cl_exportfuncs_t gClientfuncs;

void IRenderer::GetClientFuncs(cl_exportfuncs_t *pExportFuncs)
{
	memcpy(pExportFuncs, &gClientfuncs, sizeof(cl_exportfuncs_t));
}

void IRenderer::GetInterface(ref_export_t **ppRefExports, ref_funcs_t **ppRefFuncs, studio_funcs_t **ppStudioFuncs)
{
	*ppRefExports = &gRefExports;
	*ppRefFuncs = &gRefFuncs;
	*ppStudioFuncs = &gStudioFuncs;
}

EXPOSE_SINGLE_INTERFACE(IRenderer, IRenderer, RENDERER_API_VERSION);