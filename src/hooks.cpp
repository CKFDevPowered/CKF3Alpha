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

void (*g_pfnMod_LoadStudioModel)(struct model_s *mod, byte *buffer);
struct model_s *(*g_pfnCL_GetModelByIndex)(int index);
void (*g_pfnCL_AddToResourceList)(struct resource_s *pResource, struct resource_s *pList);
void (*g_pfnCL_FireEvents)(void);
struct event_hook_s *(*g_pfnCL_FindEventHook)(char *name);
void (*g_pfnInfo_SetValueForKey)(char *s, char *key, char *value, int maxsize);
int (*g_pfnLoadTGA)(const char *szFilename, byte *buffer, int bufferSize, int *width, int *height);
int (*g_pfnGL_LoadTexture)(char *identifier, int textureType, int width, int height, byte *data, qboolean mipmap, int iType, byte *pPa);
int (*g_pfnGL_LoadTexture2)(char *identifier, int textureType, int width, int height, byte *data, qboolean mipmap, int iType, byte *pPal, int filter);
void (*g_pfnGL_Bind)(int texnum);
HRESULT (CALLBACK *g_pVID_EnumDisplayModesProc)(void *lpDDSurfaceDesc, DWORD *pBPP);
CVideoMode_Common *(*g_pfnVideoMode_Create)(void);

hook_t *g_phCL_GetModelByIndex = NULL;
hook_t *g_phMod_LoadStudioModel = NULL;
hook_t *g_phCL_AddToResourceList = NULL;
hook_t *g_phCL_FireEvents = NULL;
hook_t *g_phCL_FindEventHook = NULL;
hook_t *g_phInfo_SetValueForKey = NULL;
hook_t *g_phLoadTGA = NULL;
hook_t *g_phGL_LoadTexture = NULL;
hook_t *g_phGL_LoadTexture2 = NULL;
hook_t *g_phGL_Bind = NULL;
hook_t *g_phVID_EnumDisplayModesProc = NULL;
hook_t *g_phVideoMode_Create = NULL;

bool g_bIsNewEngine = false;

void InstallHook(void)
{
	g_pfnMod_LoadStudioModel = (void (*)(struct model_s *, byte *))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, MOD_LOADSTUDIOMODEL_SIG, sizeof(MOD_LOADSTUDIOMODEL_SIG) - 1);

	if (!g_pfnMod_LoadStudioModel)
		g_pfnMod_LoadStudioModel = (void (*)(struct model_s *, byte *))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, MOD_LOADSTUDIOMODEL_SIG2, sizeof(MOD_LOADSTUDIOMODEL_SIG2) - 1);

	if (!g_pfnMod_LoadStudioModel)
	{
		g_pfnMod_LoadStudioModel = (void (*)(struct model_s *, byte *))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, MOD_LOADSTUDIOMODEL_SIG_NEW, sizeof(MOD_LOADSTUDIOMODEL_SIG_NEW) - 1);
		g_bIsNewEngine = true;
	}

	if (g_bIsNewEngine)
	{
		g_pfnGL_LoadTexture = (int (*)(char *, int, int, int, byte *, qboolean, int, byte *))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, GL_LOADTEXTURE_SIG_NEW, sizeof(GL_LOADTEXTURE_SIG_NEW) - 1);
		g_pfnCL_GetModelByIndex = (struct model_s *(*)(int))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, CL_GETMODELBYINDEX_SIG_NEW, sizeof(CL_GETMODELBYINDEX_SIG_NEW) - 1);
		g_pfnCL_AddToResourceList = (void (*)(struct resource_s *, struct resource_s *))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, CL_ADDTORESOURCELIST_SIG_NEW, sizeof(CL_ADDTORESOURCELIST_SIG_NEW) - 1);
		g_pfnCL_FireEvents = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, CL_FIREEVENTS_SIG_NEW, sizeof(CL_FIREEVENTS_SIG_NEW) - 1);
		g_pfnCL_FindEventHook = (struct event_hook_s *(*)(char *))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, CL_FINDEVENTHOOK_SIG_NEW, sizeof(CL_FINDEVENTHOOK_SIG_NEW) - 1);
		g_pfnLoadTGA = (int (*)(const char *, byte *, int, int *, int *))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, LOADTGA_SIG_NEW, sizeof(LOADTGA_SIG_NEW) - 1);
		g_pfnInfo_SetValueForKey = (void (*)(char *, char *, char *, int))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, INFO_SETVALUEFORKEY_SIG_NEW, sizeof(INFO_SETVALUEFORKEY_SIG_NEW) - 1);
		g_pfnVideoMode_Create = (CVideoMode_Common *(*)(void))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, VIDEOMODE_CREATE_SIG_NEW, sizeof(VIDEOMODE_CREATE_SIG_NEW) - 1);
	}
	else
	{
		g_pfnGL_LoadTexture = (int (*)(char *, int, int, int, byte *, qboolean, int, byte *))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, GL_LOADTEXTURE_SIG, sizeof(GL_LOADTEXTURE_SIG) - 1);
		g_pfnCL_GetModelByIndex = (struct model_s *(*)(int))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, CL_GETMODELBYINDEX_SIG, sizeof(CL_GETMODELBYINDEX_SIG) - 1);
		g_pfnCL_AddToResourceList = (void (*)(struct resource_s *, struct resource_s *))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, CL_ADDTORESOURCELIST_SIG, sizeof(CL_ADDTORESOURCELIST_SIG) - 1);
		g_pfnCL_FireEvents = (void (*)(void))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, CL_FIREEVENTS_SIG, sizeof(CL_FIREEVENTS_SIG) - 1);
		g_pfnCL_FindEventHook = (struct event_hook_s *(*)(char *))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, CL_FINDEVENTHOOK_SIG, sizeof(CL_FINDEVENTHOOK_SIG) - 1);
		g_pfnLoadTGA = (int (*)(const char *, byte *, int, int *, int *))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, LOADTGA_SIG, sizeof(LOADTGA_SIG) - 1);
		g_pfnInfo_SetValueForKey = (void (*)(char *, char *, char *, int))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, INFO_SETVALUEFORKEY_SIG, sizeof(INFO_SETVALUEFORKEY_SIG) - 1);
		g_pfnVideoMode_Create = (CVideoMode_Common *(*)(void))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, VIDEOMODE_CREATE_SIG, sizeof(VIDEOMODE_CREATE_SIG) - 1);
		g_pVID_EnumDisplayModesProc = (HRESULT (CALLBACK *)(void *, DWORD *))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, VID_ENUMDISPLAYMODESPROC_SIG, sizeof(VID_ENUMDISPLAYMODESPROC_SIG) - 1);

		if (!g_pfnVideoMode_Create)
			g_pfnVideoMode_Create = (CVideoMode_Common *(*)(void))g_pMetaHookAPI->SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, VIDEOMODE_CREATE_SIG2, sizeof(VIDEOMODE_CREATE_SIG2) - 1);
	}

	g_phCL_AddToResourceList = g_pMetaHookAPI->InlineHook(g_pfnCL_AddToResourceList, CL_AddToResourceList, (void *&)g_pfnCL_AddToResourceList);
	//g_phCL_FireEvents = g_pMetaHookAPI->InlineHook(g_pfnCL_FireEvents, CL_FireEvents, (void *&)g_pfnCL_FireEvents);
	//g_phLoadTGA = g_pMetaHookAPI->InlineHook(g_pfnLoadTGA, LoadTGA, (void *&)g_pfnLoadTGA);
	g_phInfo_SetValueForKey = g_pMetaHookAPI->InlineHook(g_pfnInfo_SetValueForKey, Info_SetValueForKey, (void *&)g_pfnInfo_SetValueForKey);
	//g_phMod_LoadStudioModel = g_pMetaHookAPI->InlineHook(g_pfnMod_LoadStudioModel, Mod_LoadStudioModel, (void *&)g_pfnMod_LoadStudioModel);
	//g_phGL_LoadTexture2 = g_pMetaHookAPI->InlineHook(g_pfnGL_LoadTexture, GL_LoadTexture, (void *&)g_pfnGL_LoadTexture);
	g_phCL_GetModelByIndex = g_pMetaHookAPI->InlineHook(g_pfnCL_GetModelByIndex, CL_GetModelByIndex, (void *&)g_pfnCL_GetModelByIndex);
	g_phVideoMode_Create = g_pMetaHookAPI->InlineHook(g_pfnVideoMode_Create, VideoMode_Create, (void *&)g_pfnVideoMode_Create);

	if (g_bIsNewEngine)
	{
	}
	else
	{
		g_phVID_EnumDisplayModesProc = g_pMetaHookAPI->InlineHook(g_pVID_EnumDisplayModesProc, VID_EnumDisplayModesProc, (void *&)g_pVID_EnumDisplayModesProc);
	}
}