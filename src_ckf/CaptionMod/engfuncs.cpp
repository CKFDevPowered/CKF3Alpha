#include <metahook.h>
#include "plugins.h"
#include "exportfuncs.h"
#include "engfuncs.h"
#include "util.h"

#include <ICKFClient.h>
#include <IRenderer.h>
#include <IBTEClient.h>

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

//Renderer.dll funcs
ref_export_t gRefExports;

//api stuffs

//Error when can't find sig

void Sys_ErrorEx(const char *fmt, ...)
{
	va_list argptr;
	char msg[1024];

	va_start(argptr, fmt);
	_vsnprintf(msg, sizeof(msg), fmt, argptr);
	va_end(argptr);

	if(g_pMetaSave->pEngineFuncs)
	{
		g_pMetaSave->pEngineFuncs->pfnClientCmd("escape\n");
	}
	MessageBox((g_dwEngineBuildnum >= 5953) ? NULL : g_pBTEClient->GetMainHWND(), msg, "Error", MB_ICONERROR);
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
	//g_pMetaHookAPI->InlineHook((void *)gHookFuncs.R_DrawViewModel, Hook_R_DrawViewModel, (void *&)gHookFuncs.R_DrawViewModel);
	g_pMetaHookAPI->InlineHook((void *)gHookFuncs.SV_StudioSetupBones, Hook_SV_StudioSetupBones, (void *&)gHookFuncs.SV_StudioSetupBones);
}

void Client_InstallHook(void)
{
}

void Renderer_Init(void)
{
	HINTERFACEMODULE hRenderer = (HINTERFACEMODULE)GetModuleHandle("Renderer.dll");
	if(!hRenderer)
		LIB_NOT_FOUND("Renderer.dll");

	IRenderer *pRenderer = (IRenderer *)((CreateInterfaceFn)Sys_GetFactory(hRenderer))(RENDERER_API_VERSION, NULL);

	if(!pRenderer)
		LIB_NOT_FOUND("Renderer.dll");

	pRenderer->GetInterface(&gRefExports, META_RENDERER_VERSION);
}

extern float *ev_punchangle;

void BTE_Init(void)
{
	HINTERFACEMODULE hBTEClient = (HINTERFACEMODULE)GetModuleHandle("CSBTE.dll");
	if(!hBTEClient)
		LIB_NOT_FOUND("CSBTE.dll");

	g_pBTEClient = (IBTEClient *)((CreateInterfaceFn)Sys_GetFactory(hBTEClient))(BTECLIENT_API_VERSION, NULL);

	ev_punchangle = g_pBTEClient->GetPunchAngles();
}