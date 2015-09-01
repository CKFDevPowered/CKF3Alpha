#ifndef _METAHOOK_H
#define _METAHOOK_H

#ifndef EXPORT
#define EXPORT __declspec(dllexport)
#endif

#include <winsani_in.h>
#include <windows.h>
#include <winsani_out.h>
#include <stdio.h>

typedef float vec_t;
typedef float vec2_t[2];
typedef float vec3_t[3];
typedef float vec4_t[4];

#include <wrect.h>
#include <interface.h>

typedef int (*pfnUserMsgHook)(const char *pszName, int iSize, void *pbuf);

#include <cdll_export.h>
#include <cdll_int.h>

typedef struct hook_s hook_t;

#define VIDEOMODE_SOFTWARE 0
#define VIDEOMODE_OPENGL 1
#define VIDEOMODE_D3D 2

typedef struct metahook_api_s
{
	BOOL (*UnHook)(hook_t *pHook);
	hook_t *(*InlineHook)(void *pOldFuncAddr, void *pNewFuncAddr, void *&pCallBackFuncAddr);
	hook_t *(*VFTHook)(void *pClass, int iTableIndex, int iFuncIndex, void *pNewFuncAddr, void *&pCallBackFuncAddr);
	hook_t *(*IATHook)(HMODULE hModule, const char *pszModuleName, const char *pszFuncName, void *pNewFuncAddr, void *&pCallBackFuncAddr);
	void *(*GetClassFuncAddr)(...);
	DWORD (*GetModuleBase)(HMODULE hModule);
	DWORD (*GetModuleSize)(HMODULE hModule);
	HMODULE (*GetEngineModule)(void);
	DWORD (*GetEngineBase)(void);
	DWORD (*GetEngineSize)(void);
	void *(*SearchPattern)(void *pStartSearch, DWORD dwSearchLen, char *pPattern, DWORD dwPatternLen);
	void (*WriteDWORD)(void *pAddress, DWORD dwValue);
	DWORD (*ReadDWORD)(void *pAddress);
	DWORD (*WriteMemory)(void *pAddress, BYTE *pData, DWORD dwDataSize);
	DWORD (*ReadMemory)(void *pAddress, BYTE *pData, DWORD dwDataSize);
	DWORD (*GetVideoMode)(int *width, int *height, int *bpp, bool *windowed);
	DWORD (*GetEngineBuildnum)(void);
	CreateInterfaceFn (*GetEngineFactory)(void);
	DWORD (*GetNextCallAddr)(void *pAddress, DWORD dwCount);
	void (*WriteBYTE)(void *pAddress, BYTE ucValue);
	BYTE (*ReadBYTE)(void *pAddress);
	void (*WriteNOP)(void *pAddress, DWORD dwCount);
}
metahook_api_t;

typedef struct mh_enginesave_s
{
	cl_exportfuncs_t *pExportFuncs;
	cl_enginefunc_t *pEngineFuncs;
}
mh_enginesave_t;

void MH_ShutdownHooks(void);
void MH_LoadLibrary(const char *pszDLLPath);
void MH_FreeLibrary(void);

extern "C"
{
	EXPORT void MH_Setup(class IFileSystem *pFileSystem, class ICommandLine *pCommandLine, class IRegistry *pRegistry);
	EXPORT void MH_Init(const char *pszGameName);
	EXPORT void MH_SetBlobHeader(struct BlobHeader_s *pHeader);
	EXPORT void MH_LoadClient(cl_exportfuncs_t *pExportFuncs, cl_exportfuncs_t *pCallExportFuncs);
	EXPORT void MH_LoadEngine(HMODULE hModule);
	EXPORT void MH_ExitGame(int iResult);
	EXPORT void MH_Shutdown(void);
}

#include <IFileSystem.h>
#include <ICommandLine.h>
#include <IRegistry.h>

typedef struct mh_interface_s
{
	ICommandLine *CommandLine;
	IFileSystem *FileSystem;
	IRegistry *Registry;
}
mh_interface_t;

#include <IPlugins.h>

extern mh_interface_t *g_pInterface;
extern cl_enginefunc_t gEngfuncs;
extern metahook_api_t *g_pMetaHookAPI;
extern mh_enginesave_t *g_pMetaSave;
#endif