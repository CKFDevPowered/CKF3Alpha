#include <metahook.h>
#include "plugins.h"
#include "exportfuncs.h"
#include "engfuncs.h"
#include "util.h"

#include <ICKFClient.h>
#include <IRenderer.h>

hook_funcs_t gHookFuncs;

//engine

//3266
#define SV_STUDIOSETUPBONE_SIG "\x8B\x4C\x24\x0C\xA1\x2A\x2A\x2A\x02\x81\xEC\x70\x02\x00\x00\x53\x55"
//4554
#define SV_STUDIOSETUPBONE_SIG2 "\x8B\x4C\x24\x0C\x81\xEC\x74\x02\x00\x00\xA1\x2A\x2A\x2A\x2A\x53\x55"
//6153
#define SV_STUDIOSETUPBONE_SIG_NEW "\x55\x8B\xEC\x81\xEC\x64\x02\x00\x00\x8B\x4D\x10\xA1\x2A\x2A\x2A\x2A\x53\x33\xDB\x56\x57\x85\xC9"

//3266,4554
#define R_DRAWVIEWMODEL_SIG "\x83\xEC\x2A\xD9\x05\x2A\x2A\x2A\x2A\xD8\x1D\x2A\x2A\x2A\x2A\x56\x57\x33\xFF\xC7\x44\x2A\x2A\x00\x00\x80\xBF"
//6153
#define R_DRAWVIEWMODEL_SIG_NEW "\x55\x8B\xEC\x83\xEC\x50\xD9\x05\x2A\x2A\x2A\x2A\xD8\x1D\x2A\x2A\x2A\x2A\x56\x57\x33\xFF\xC7\x45\xE0\x00\x00\x80\xBF"

#define S_STOPSOUND_SIG "\xA1\x2A\x2A\x2A\x2A\x57\xBF\x04\x00\x00\x00\x3B\xC7\x7E\x35\x53\x8B\x5C\x24\x10\x55\x8B\x6C\x24\x10"
#define S_STOPSOUND_SIG_NEW "\x55\x8B\xEC\xA1\x2A\x2A\x2A\x2A\x57\xBF\x04\x00\x00\x00\x3B\xC7\x7E\x2A\x53\x8B\x5D\x0C\x56"

//client
#define IN_MOUSEMOVE_SIG "\x83\xEC\x2A\x8D\x44\x24\x2A\x50\xFF\x15\x2A\x2A\x2A\x2A\xA0\x2A\x2A\x2A\x2A\x83\xC4\x04\xA8\x01"

//client.dll vars
int g_iUser1 = 0;
int g_iUser2 = 0;
int g_iUser3 = 0;
cvar_t *gHUD_m_pip = NULL;

//hw.dll vars
refdef_t *refdef = NULL;
playermove_t *cl_pmove = NULL;
int *envmap = NULL;

//Renderer.dll csbte.dll funcs
ref_export_t *gpRefExports;
ref_funcs_t *gpRefFuncs;
studio_funcs_t *gpStudioFuncs;
bte_funcs_t gBTEFuncs;

//api stuffs
HWND g_hWnd;

//Error when can't find sig
void Sys_ErrorEx(const char *error)
{
	if(g_pMetaSave->pEngineFuncs)
	{
		g_pMetaSave->pEngineFuncs->pfnClientCmd("escape\n");
	}
	MessageBox(g_hWnd, error, "Error", MB_ICONERROR);
	exit(0);
}

void Engine_InstallHook(void)
{
	DWORD addr;

	if(g_dwEngineBuildnum >= 5953)
	{
		gHookFuncs.SV_StudioSetupBones = (void (*)(model_t *, float , int , vec_t *, vec_t *, const byte *, const byte *, int , const edict_t *))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, SV_STUDIOSETUPBONE_SIG_NEW, sizeof(SV_STUDIOSETUPBONE_SIG_NEW)-1);
		if(!gHookFuncs.SV_StudioSetupBones)
			SIG_NOT_FOUND("SV_StudioSetupBones");

		gHookFuncs.R_DrawViewModel = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, R_DRAWVIEWMODEL_SIG_NEW, sizeof(R_DRAWVIEWMODEL_SIG_NEW)-1);
		if(!gHookFuncs.R_DrawViewModel)
			SIG_NOT_FOUND("R_DrawViewModel");

		gHookFuncs.S_StopSound = (void (*)(int, int))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, S_STOPSOUND_SIG_NEW, sizeof(S_STOPSOUND_SIG_NEW)-1);
		if(!gHookFuncs.S_StopSound)
			SIG_NOT_FOUND("S_StopSound");
	}
	else
	{
		gHookFuncs.SV_StudioSetupBones = (void (*)(model_t *, float , int , vec_t *, vec_t *, const byte *, const byte *, int , const edict_t *))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, SV_STUDIOSETUPBONE_SIG, sizeof(SV_STUDIOSETUPBONE_SIG)-1);
		if(!gHookFuncs.SV_StudioSetupBones)
			gHookFuncs.SV_StudioSetupBones = (void (*)(model_t *, float , int , vec_t *, vec_t *, const byte *, const byte *, int , const edict_t *))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, SV_STUDIOSETUPBONE_SIG2, sizeof(SV_STUDIOSETUPBONE_SIG2)-1);
		if(!gHookFuncs.SV_StudioSetupBones)
			SIG_NOT_FOUND("SV_StudioSetupBones");

		gHookFuncs.R_DrawViewModel = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, R_DRAWVIEWMODEL_SIG, sizeof(R_DRAWVIEWMODEL_SIG)-1);
		if(!gHookFuncs.R_DrawViewModel)
			SIG_NOT_FOUND("R_DrawViewModel");

		gHookFuncs.S_StopSound = (void (*)(int, int))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, S_STOPSOUND_SIG, sizeof(S_STOPSOUND_SIG)-1);
		if(!gHookFuncs.S_StopSound)
			SIG_NOT_FOUND("S_StopSound");
	}

	//cmp     dword ptr envmap, edi
	//jnz
	//fld     r_drawentities.value
#define ENVMAP_SIG "\x39\x3D\x2A\x2A\x2A\x2A\x0F\x85\x2A\x2A\x2A\x2A\xD9\x05"
	addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gHookFuncs.R_DrawViewModel, 0x200, ENVMAP_SIG, sizeof(ENVMAP_SIG)-1);
	if(!addr)
		SIG_NOT_FOUND("envmap");
	envmap = *(int **)(addr + 2);

	//hook funcs at the end, just in case
	g_pMetaHookAPI->InlineHook((void *)gHookFuncs.R_DrawViewModel, Hook_R_DrawViewModel, (void *&)gHookFuncs.R_DrawViewModel);
	g_pMetaHookAPI->InlineHook((void *)gHookFuncs.SV_StudioSetupBones, Hook_SV_StudioSetupBones, (void *&)gHookFuncs.SV_StudioSetupBones);
}

void API_InstallHook(void)
{
	HMODULE hUser32 = GetModuleHandle("user32.dll");
	
	g_pMetaHookAPI->InlineHook(GetProcAddress(hUser32, "SetWindowTextA"), Hook_SetWindowTextA, (void *&)gHookFuncs.SetWindowTextA);
	//g_pMetaHookAPI->InlineHook(GetProcAddress(hUser32, "SetCursorPos"), Hook_SetCursorPos, (void *&)gHookFuncs.SetCursorPos);
	g_pMetaHookAPI->InlineHook(GetProcAddress(hUser32, "CreateWindowExA"), Hook_CreateWindowExA, (void *&)gHookFuncs.CreateWindowExA);
	g_pMetaHookAPI->InlineHook(GetProcAddress(hUser32, "CreateWindowExW"), Hook_CreateWindowExW, (void *&)gHookFuncs.CreateWindowExW);
}

void Client_InstallHook(void)
{
	//DWORD addr;

	//gHookFuncs.IN_MouseMove = (void (*)(float, usercmd_t *))g_pMetaHookAPI->SearchPattern((void *)g_pMetaSave->pExportFuncs->IN_MouseEvent, 0x300, IN_MOUSEMOVE_SIG, sizeof(IN_MOUSEMOVE_SIG)-1);
	//if(!gHookFuncs.IN_MouseMove)
	//	SIG_NOT_FOUND("IN_MouseMove");

	//g_pMetaHookAPI->InlineHook((void *)gHookFuncs.IN_MouseMove, Hook_IN_MouseMove, (void *&)gHookFuncs.IN_MouseMove);
}

void Renderer_Init(void)
{
	HINTERFACEMODULE hRenderer = Sys_LoadModule("Renderer.dll");
	if(!hRenderer)
		LIB_NOT_FOUND("Renderer.dll");

	IRenderer *pRenderer = (IRenderer *)((CreateInterfaceFn)Sys_GetFactory(hRenderer))(RENDERER_API_VERSION, NULL);

	if(!pRenderer)
		LIB_NOT_FOUND("Renderer.dll");

	pRenderer->GetInterface(&gpRefExports, &gpRefFuncs, &gpStudioFuncs);

	refdef = gpRefExports->R_GetRefDef();
}