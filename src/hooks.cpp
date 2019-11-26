#include <metahook.h>
#include "plugins.h"
#include "client.h"
#include "model.h"
#include "textures.h"
#include "LoadTGA.h"
#include "info.h"
#include "vgui_internal.h"
#include "sys.h"
#include "Video.h"
#include "VideoMode.h"
#include "hooks.h"

#define GetCallAddress(addr) (addr + (*(DWORD *)(addr+1)) + 5)

#define LOADTGA_SIG "\x8B\x44\x24\x14\x8B\x4C\x24\x10\x8B\x54\x24\x0C\x6A\x01\x50\x8B\x44\x24\x10\x51\x8B\x4C\x24\x10\x52\x50\x51\xE8\x2A\x2A\x2A\x2A\x83\xC4\x18\xC3"
#define LOADTGA_SIG_NEW "\x55\x8B\xEC\x8B\x45\x18\x8B\x4D\x14\x8B\x55\x10\x6A\x00\x50\x8B\x45\x0C\x51\x8B\x4D\x08\x52\x50\x51\xE8\x2A\x2A\x2A\x2A\x83\xC4\x18\x5D\xC3"
#define VIDEOMODE_CREATE_SIG "\x83\xEC\x08\x8B\x0D\x2A\x2A\x2A\x2A\x53\x56\x33\xF6\x8B\x01\x56\x68\x2A\x2A\x2A\x2A\xFF\x50\x08"
#define VIDEOMODE_CREATE_SIG2 "\x8B\x0D\x2A\x2A\x2A\x2A\x53\x6A\x00\x68\x2A\x2A\x2A\x2A\x8B\x01\xFF\x50\x08\x85\xC0\x68"
#define VIDEOMODE_CREATE_SIG_NEW "\x55\x8B\xEC\x51\x8B\x0D\x2A\x2A\x2A\x2A\x53\x6A\x00\x68\x2A\x2A\x2A\x2A\x8B\x01\xFF\x50\x08"
#define VID_ENUMDISPLAYMODESPROC_SIG "\x8B\x4C\x24\x04\x53\x56\x57\x8B\x51\x08\x8B\x41\x0C\x8B\x71\x54\x8B\xFA\xC1\xE7\x04\x8D\x1C\xC0\x32\xC9\x3B\xDF"
#define MOD_LOADSTUDIOMODEL_SIG "\x81\xEC\x0C\x01\x00\x00\x53\x8B\x9C\x24\x18\x01\x00\x00\x2A\x2A\x8B\x43\x04"
#define MOD_LOADSTUDIOMODEL_SIG2 "\x55\x8B\xEC\x83\xEC\x08\x53\x8B\x5D\x0C\x56\x57\x8B\x43\x04\x50\xFF\x15"
#define MOD_LOADSTUDIOMODEL_SIG_NEW "\x55\x8B\xEC\x81\xEC\x0C\x01\x00\x00\x53\x8B\x5D\x0C\x56\x57\x8B\x43\x04"
#define GL_LOADTEXTURE_SIG "\xA1\x2A\x2A\x2A\x2A\x8B\x4C\x24\x20\x8B\x54\x24\x1C\x50\x8B\x44\x24\x1C\x51\x8B\x4C\x24\x1C\x52\x8B\x54\x24\x1C\x50\x8B\x44\x24\x1C"
#define GL_LOADTEXTURE_SIG_NEW "\x55\x8B\xEC\xA1\x2A\x2A\x2A\x2A\x8B\x4D\x24\x8B\x55\x20\x50\x8B\x45\x1C\x51\x8B\x4D\x18\x52\x8B\x55\x14\x50\x8B\x45\x10\x51"
#define GL_LOADTEXTURE2_SIG "\xB8\x0C\x40\x00\x00\xE8\x2A\x2A\x2A\x2A\x53\x8B\x9C\x24\x14\x40\x00\x00\x55\x56\x8A\x03\x33\xF6"
#define GL_LOADTEXTURE2_SIG2 "\xB8\x0C\x40\x00\x00\xE8\x2A\x2A\x2A\x2A\x8B\x84\x24\x10\x40\x00\x00\x53\x55\x33\xDB\x8A\x08\x56\x84\xC9\x57\x89\x5C\x24\x14"
#define GL_LOADTEXTURE2_SIG_NEW "\x55\x8B\xEC\xB8\x0C\x40\x00\x00\xE8\x2A\x2A\x2A\x2A\x8B\x45\x08\x53\x33\xDB\x56\x8A\x08\x57\x84\xC9\x89\x5D\xF4\x74\x2A\x33\xF6"
#define CL_GETMODELBYINDEX_SIG "\x83\xEC\x10\x56\x57\x8B\x7C\x24\x1C\x8B\x34\xBD\x2A\x2A\x2A\x2A\x85\xF6"
#define CL_GETMODELBYINDEX_SIG_NEW "\x55\x8B\xEC\x83\xEC\x10\x56\x57\x8B\x7D\x08\x81\xFF\x00\x02\x00\x00\x7C\x2A\x5F\x33\xC0\x5E\x8B\xE5\x5D\xC3\x8B\x34\xBD\x2A\x2A\x2A\x2A\x85\xF6"
#define CL_ADDTORESOURCELIST_SIG "\x8B\x44\x24\x04\x8B\x88\x84\x00\x00\x00\x85\xC9"
#define CL_ADDTORESOURCELIST_SIG_NEW "\x55\x8B\xEC\x8B\x45\x08\x8B\x88\x84\x00\x00\x00\x85\xC9\x75\x2A\x8B\x88\x80\x00\x00\x00\x85\xC9"
#define CL_FIREEVENTS_SIG "\x53\x56\x57\x33\xDB\xBE\x2A\x2A\x2A\x2A\x66\x8B\x0E\x66\x85\xC9\x0F\x84\x2A\x2A\x2A\x2A\xD9\x46\x08\xD8\x1D\x2A\x2A\x2A\x2A\xDF\xE0\xF6\xC4"
#define CL_FIREEVENTS_SIG_NEW "\x53\x56\x57\x33\xDB\xBE\x2A\x2A\x2A\x2A\x66\x8B\x0E\x66\x85\xC9\x0F\x84\x2A\x2A\x2A\x2A\xD9\x46\x08\xD8\x1D\x2A\x2A\x2A\x2A\xDF\xE0\xF6\xC4"
#define CL_FINDEVENTHOOK_SIG "\x56\x8B\x35\x2A\x2A\x2A\x2A\x85\xF6\x57\x74\x2A\x8B\x7C\x24\x0C\x8B\x46\x04\x50\x57\xE8\x2A\x2A\x2A\x2A\x83\xC4\x08\x85\xC0\x74"
#define CL_FINDEVENTHOOK_SIG_NEW "\x55\x8B\xEC\x56\x8B\x35\x2A\x2A\x2A\x2A\x57\x8B\x7D\x08\x85\xFF\x74\x2A\x85\xF6\x74\x2A\x8B\x46\x04\x85\xC0\x74\x2A\x50\x57\xE8\x2A\x2A\x2A\x2A\x83\xC4\x08\x85\xC0\x74"
#define INFO_SETVALUEFORKEY_SIG "\x8B\x44\x24\x08\x80\x38\x2A\x2A\x2A\x68\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x83\xC4\x04\xC3"
#define INFO_SETVALUEFORKEY_SIG_NEW "\x55\x8B\xEC\x8B\x45\x0C\x80\x38\x2A\x75\x2A\x68\x2A\x2A\x2A\x2A\xE8\x2A\x2A\x2A\x2A\x83\xC4\x04\x5D\xC3"

//3266
#define LOADSTARTUPGRAPHIC_SIG "\x55\x8B\xEC\x81\xEC\x08\x02\x00\x00\x53\x56\x57\x68\x2A\x2A\x2A\x2A\x68"
//4554
#define LOADSTARTUPGRAPHIC_SIG2 "\x55\x8B\xEC\x81\xEC\x2A\x02\x00\x00\x53\x56\x57\x8B\xF1\x68\x2A\x2A\x2A\x2A\x68"
//6153
#define LOADSTARTUPGRAPHIC_SIG_NEW "\x55\x8B\xEC\x81\xEC\x0C\x02\x00\x00\x53\x56\x57\x8B\xF1\x68\x2A\x2A\x2A\x2A\x68"
//3266,4554
#define DRAWSTARTUPGRAPHIC_SIG "\x55\x8B\xEC\x83\xE4\xF8\x83\xEC\x2C\x53\x56\x57"
//6153
#define DRAWSTARTUPGRAPHIC_SIG_NEW "\x55\x8B\xEC\x83\xEC\x78\x53\x56\x8B\xF1"

//3266,4554
#define CL_READCLIENTDLLDATA_SIG "\xB8\x04\x80\x00\x00\xE8\x2A\x2A\x2A\x2A\x68\x00\x80\x00\x00\x8D\x44\x24\x08\x6A\x00\x50"
//6153
#define CL_READCLIENTDLLDATA_SIG_NEW "\x55\x8B\xEC\xB8\x04\x80\x00\x00\xE8\x2A\x2A\x2A\x2A\x68\x00\x80\x00\x00\x8D\x85\xFC\x7F\xFF\xFF\x6A\x00\x50"

//3266,4554
#define CL_DEMOPARSESOUND_SIG "\x81\xEC\x10\x01\x00\x00\xA1\x2A\x2A\x2A\x2A\x56\x57\x50\x6A\x01"
//6153
#define CL_DEMOPARSESOUND_SIG_NEW "\x55\x8B\xEC\x81\xEC\x10\x01\x00\x00\xA1\x2A\x2A\x2A\x2A\x56\x57\x50\x6A\x01"

void Sys_ErrorEx(const char *fmt, ...)
{
	va_list argptr;
	char msg[1024];

	va_start(argptr, fmt);
	_vsnprintf(msg, sizeof(msg), fmt, argptr);
	va_end(argptr);

	if(g_pMetaSave->pEngineFuncs)
		g_pMetaSave->pEngineFuncs->pfnClientCmd("escape\n");

	MessageBox(NULL, msg, "Error", MB_ICONERROR);
	exit(0);
}

#define SIG_NOT_FOUND(name) Sys_ErrorEx("Could not found: %s\nEngine buildnum: %d", name, g_dwEngineBuildnum);

CVideoMode_Common **g_pVideoMode = NULL;
IGame **g_pGame = NULL;

HookFuncs_t gHookFuncs;

void InstallHook(void)
{
	if (g_dwEngineBuildnum >= 5953)
	{
		gHookFuncs.CL_GetModelByIndex = (struct model_s *(*)(int))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, CL_GETMODELBYINDEX_SIG_NEW, sizeof(CL_GETMODELBYINDEX_SIG_NEW) - 1);
		if(!gHookFuncs.CL_GetModelByIndex)
			SIG_NOT_FOUND("CL_GetModelByIndex");

		gHookFuncs.CL_AddToResourceList = (void (*)(struct resource_s *, struct resource_s *))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, CL_ADDTORESOURCELIST_SIG_NEW, sizeof(CL_ADDTORESOURCELIST_SIG_NEW) - 1);
		if(!gHookFuncs.CL_AddToResourceList)
			SIG_NOT_FOUND("CL_AddToResourceList");
		
		gHookFuncs.CL_FindEventHook = (struct event_hook_s *(*)(char *))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, CL_FINDEVENTHOOK_SIG_NEW, sizeof(CL_FINDEVENTHOOK_SIG_NEW) - 1);
		if(!gHookFuncs.CL_FindEventHook)
			SIG_NOT_FOUND("CL_FindEventHook");
		
		gHookFuncs.Info_SetValueForKey = (void (*)(char *, char *, char *, int))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, INFO_SETVALUEFORKEY_SIG_NEW, sizeof(INFO_SETVALUEFORKEY_SIG_NEW) - 1);
		if(!gHookFuncs.Info_SetValueForKey)
			SIG_NOT_FOUND("Info_SetValueForKey");
		
		gHookFuncs.VideoMode_Create = (CVideoMode_Common *(*)(void))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, VIDEOMODE_CREATE_SIG_NEW, sizeof(VIDEOMODE_CREATE_SIG_NEW) - 1);
		if(!gHookFuncs.VideoMode_Create)
			SIG_NOT_FOUND("VideoMode_Create");

		gHookFuncs.LoadStartupGraphic = (void (__fastcall *)(void *, int))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, LOADSTARTUPGRAPHIC_SIG_NEW, sizeof(LOADSTARTUPGRAPHIC_SIG_NEW)-1);
		if(!gHookFuncs.LoadStartupGraphic)
			SIG_NOT_FOUND("LoadStartupGraphic");

		gHookFuncs.DrawStartupGraphic = (void (__fastcall *)(void *, int, HWND))g_pMetaHookAPI->SearchPattern((void *)gHookFuncs.LoadStartupGraphic, g_dwEngineSize - (DWORD)gHookFuncs.LoadStartupGraphic + g_dwEngineBase, DRAWSTARTUPGRAPHIC_SIG_NEW, sizeof(DRAWSTARTUPGRAPHIC_SIG_NEW)-1);
		if(!gHookFuncs.DrawStartupGraphic)
			SIG_NOT_FOUND("DrawStartupGraphic");

		gHookFuncs.CL_ReadClientDLLData = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, CL_READCLIENTDLLDATA_SIG_NEW, sizeof(CL_READCLIENTDLLDATA_SIG_NEW)-1);
		gHookFuncs.CL_DemoParseSound = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, CL_DEMOPARSESOUND_SIG_NEW, sizeof(CL_DEMOPARSESOUND_SIG_NEW)-1);
	}
	else
	{
		gHookFuncs.CL_GetModelByIndex = (struct model_s *(*)(int))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, CL_GETMODELBYINDEX_SIG, sizeof(CL_GETMODELBYINDEX_SIG) - 1);
		if(!gHookFuncs.CL_GetModelByIndex)
			SIG_NOT_FOUND("CL_GetModelByIndex");

		gHookFuncs.CL_AddToResourceList = (void (*)(struct resource_s *, struct resource_s *))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, CL_ADDTORESOURCELIST_SIG, sizeof(CL_ADDTORESOURCELIST_SIG) - 1);
		if(!gHookFuncs.CL_AddToResourceList)
			SIG_NOT_FOUND("CL_AddToResourceList");

		gHookFuncs.CL_FindEventHook = (struct event_hook_s *(*)(char *))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, CL_FINDEVENTHOOK_SIG, sizeof(CL_FINDEVENTHOOK_SIG) - 1);
		if(!gHookFuncs.CL_FindEventHook)
			SIG_NOT_FOUND("CL_FindEventHook");

		gHookFuncs.Info_SetValueForKey = (void (*)(char *, char *, char *, int))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, INFO_SETVALUEFORKEY_SIG, sizeof(INFO_SETVALUEFORKEY_SIG) - 1);
		if(!gHookFuncs.Info_SetValueForKey)
			SIG_NOT_FOUND("Info_SetValueForKey");

		gHookFuncs.VideoMode_Create = (CVideoMode_Common *(*)(void))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, VIDEOMODE_CREATE_SIG, sizeof(VIDEOMODE_CREATE_SIG) - 1);
		if (!gHookFuncs.VideoMode_Create)
			gHookFuncs.VideoMode_Create = (CVideoMode_Common *(*)(void))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, VIDEOMODE_CREATE_SIG2, sizeof(VIDEOMODE_CREATE_SIG2) - 1);
		if(!gHookFuncs.VideoMode_Create)
			SIG_NOT_FOUND("VideoMode_Create");

		gHookFuncs.VID_EnumDisplayModesProc = (HRESULT (CALLBACK *)(void *, DWORD *))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, VID_ENUMDISPLAYMODESPROC_SIG, sizeof(VID_ENUMDISPLAYMODESPROC_SIG) - 1);
		if(!gHookFuncs.VID_EnumDisplayModesProc)
			SIG_NOT_FOUND("VID_EnumDisplayModesProc");

		gHookFuncs.LoadStartupGraphic = (void (__fastcall *)(void *, int))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, LOADSTARTUPGRAPHIC_SIG, sizeof(LOADSTARTUPGRAPHIC_SIG)-1);
		if(!gHookFuncs.LoadStartupGraphic)
			gHookFuncs.LoadStartupGraphic = (void (__fastcall *)(void *, int))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, LOADSTARTUPGRAPHIC_SIG2, sizeof(LOADSTARTUPGRAPHIC_SIG2)-1);
		if(!gHookFuncs.LoadStartupGraphic)
			SIG_NOT_FOUND("LoadStartupGraphic");

		gHookFuncs.DrawStartupGraphic = (void (__fastcall *)(void *, int, HWND))g_pMetaHookAPI->SearchPattern((void *)gHookFuncs.LoadStartupGraphic, g_dwEngineSize - (DWORD)gHookFuncs.LoadStartupGraphic + g_dwEngineBase, DRAWSTARTUPGRAPHIC_SIG, sizeof(DRAWSTARTUPGRAPHIC_SIG)-1);
		if(!gHookFuncs.DrawStartupGraphic)
			SIG_NOT_FOUND("DrawStartupGraphic");

		gHookFuncs.CL_ReadClientDLLData = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, CL_READCLIENTDLLDATA_SIG, sizeof(CL_READCLIENTDLLDATA_SIG)-1);
		gHookFuncs.CL_DemoParseSound = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, CL_DEMOPARSESOUND_SIG, sizeof(CL_DEMOPARSESOUND_SIG)-1);
	}

	DWORD addr;

	if(gHookFuncs.CL_ReadClientDLLData && gHookFuncs.CL_DemoParseSound)
	{
		#define CLS_DEMOFILE_SIG "\x00\x00\xA1"
		addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gHookFuncs.CL_DemoParseSound, 0x10, CLS_DEMOFILE_SIG, sizeof(CLS_DEMOFILE_SIG)-1);
		gHookFuncs.cls_demofile = *(FileHandle_t **)(addr + 3);

		#define CL_DEMOPLAYSOUND_SIG "\x52\x50\x56\xE8\x2A\x2A\x2A\x2A\x83\xC4\x18"
        #define CL_DEMOPLAYSOUND_8308_SIG "\x52\x50\x57\xE8\x2A\x2A\x2A\x2A\x83\xC4\x30"
		addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gHookFuncs.CL_DemoParseSound, 0x200, CL_DEMOPLAYSOUND_SIG, sizeof(CL_DEMOPLAYSOUND_SIG)-1);
        if (!addr)
            addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gHookFuncs.CL_DemoParseSound, 0x200, CL_DEMOPLAYSOUND_8308_SIG, sizeof(CL_DEMOPLAYSOUND_8308_SIG) - 1);
		gHookFuncs.CL_DemoPlaySound = (void (*)( int, char *, float, float, int, int ))GetCallAddress(addr + 3);

		g_pMetaHookAPI->InlineHook(gHookFuncs.CL_ReadClientDLLData, CL_ReadClientDLLData, (void *&)gHookFuncs.CL_ReadClientDLLData);
		g_pMetaHookAPI->InlineHook(gHookFuncs.CL_DemoParseSound, CL_DemoParseSound, (void *&)gHookFuncs.CL_DemoParseSound);
	}

	g_pMetaHookAPI->InlineHook(gHookFuncs.CL_AddToResourceList, CL_AddToResourceList, (void *&)gHookFuncs.CL_AddToResourceList);
	g_pMetaHookAPI->InlineHook(gHookFuncs.Info_SetValueForKey, Info_SetValueForKey, (void *&)gHookFuncs.Info_SetValueForKey);
	//g_pMetaHookAPI->InlineHook(gHookFuncs.CL_GetModelByIndex, CL_GetModelByIndex, (void *&)gHookFuncs.CL_GetModelByIndex);

	//E8 83 FA FF FF		call    VideoMode_Create
	//5E					pop     esi
	//A3 5C 05 3C 02		mov     videomode, eax
#define IVIDEOMODE_SIG "\xE8\x2A\x2A\x2A\x2A\x5E\xA3"

	//E8 83 FA FF FF		call    VideoMode_Create
	//A3 5C 05 3C 02		mov     videomode, eax
#define IVIDEOMODE_SIG_NEW "\xE8\x2A\x2A\x2A\x2A\xA3"

	addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gHookFuncs.VideoMode_Create, 0x200, IVIDEOMODE_SIG, sizeof(IVIDEOMODE_SIG) - 1);
	if(addr)
	{
		g_pVideoMode = *(CVideoMode_Common ***)(addr + 7);
	}
	else
	{
		addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gHookFuncs.VideoMode_Create, 0x200, IVIDEOMODE_SIG_NEW, sizeof(IVIDEOMODE_SIG_NEW) - 1);
		g_pVideoMode = *(CVideoMode_Common ***)(addr + 6);
	}

	void (__fastcall *DrawStartupGraphic)(void *, int, HWND);

	if(g_dwEngineBuildnum < 5953)
		DrawStartupGraphic = DrawStartupGraphic_GDI;
	else
		DrawStartupGraphic = DrawStartupGraphic_GL;

	if(g_dwEngineBuildnum >= 5953)
	{
	#define SDL_GETWINDOWSIZE_SIG "\x50\x51\x52\xE8"
		addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gHookFuncs.DrawStartupGraphic, 0x50, SDL_GETWINDOWSIZE_SIG, sizeof(SDL_GETWINDOWSIZE_SIG)-1);
		if(!addr)
			SIG_NOT_FOUND("SDL_GetWindowSize");
		gHookFuncs.SDL_GetWindowSize = (void (*)(HWND, int *, int *))GetCallAddress(addr+3);
#define SDL_SWAPBUFFER_SIG "\x8B\x45\x08\x50\xE8"
		addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)gHookFuncs.DrawStartupGraphic, 0x600, SDL_SWAPBUFFER_SIG, sizeof(SDL_SWAPBUFFER_SIG)-1);
		if(!addr)
			SIG_NOT_FOUND("SDL_SwapBuffer");
		gHookFuncs.SDL_SwapBuffer = (void (*)(HWND))GetCallAddress(addr+4);
	}

	if(*g_pVideoMode == NULL)
	{
		g_pMetaHookAPI->InlineHook((void *)gHookFuncs.LoadStartupGraphic, LoadStartupGraphic, (void *&)gHookFuncs.LoadStartupGraphic);
		g_pMetaHookAPI->InlineHook((void *)gHookFuncs.DrawStartupGraphic, DrawStartupGraphic, (void *&)gHookFuncs.DrawStartupGraphic);
		g_pMetaHookAPI->InlineHook(gHookFuncs.VideoMode_Create, VideoMode_Create, (void *&)gHookFuncs.VideoMode_Create);
	}
	else
	{
		//the CVideoModeCommon::Init is already called, so we could get main window and draw the loading screen right now
	#define IGAME_SIG "\x8B\x0D"
		DWORD *pVFTable = *(DWORD **)(*g_pVideoMode);
		addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)pVFTable[10], 0x20, IGAME_SIG, sizeof(IGAME_SIG) - 1);
		g_pGame = *(IGame ***)(addr + 2);

		HWND hWnd = (*g_pGame)->GetMainWindow();
		//MessageBox(NULL, "", "", 0);
		if(!g_hMainWnd && hWnd)
		{
			VID_SetMainWindow(hWnd);
		}
		LoadStartupGraphic(*g_pVideoMode, 0);
		DrawStartupGraphic(*g_pVideoMode, 0, hWnd);
	}
	if (g_dwEngineBuildnum >= 5953)
	{
	}
	else
	{
		g_pMetaHookAPI->InlineHook(gHookFuncs.VID_EnumDisplayModesProc, VID_EnumDisplayModesProc, (void *&)gHookFuncs.VID_EnumDisplayModesProc);
	}
}