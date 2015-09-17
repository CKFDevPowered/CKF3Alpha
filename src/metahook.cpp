#include "metahook.h"
#include "Detours\detours.h"
#include "resource.h"
#include "interface.h"
#include "LoadBlob.h"
#include "memdll.h"
#include <imagehlp.h>

#include <IPluginsV1.h>
#include <IEngine.h>

#pragma comment(lib, "Dbghelp.lib")

struct hook_s
{
	void *pOldFuncAddr;
	void *pNewFuncAddr;
	void *pClass;
	int iTableIndex;
	int iFuncIndex;
	HMODULE hModule;
	const char *pszModuleName;
	const char *pszFuncName;
	struct hook_s *pNext;
	void *pInfo;
};

int (*g_pfnbuild_number)(void);
void *g_pClientDLL_Init;
int (*g_pfnClientDLL_Init)(void);
hook_t *g_phClientDLL_Init;
DWORD g_dwClientDLL_Init_CallAddress;

BOOL g_bEngineIsBlob;
HMODULE g_hEngineModule;
DWORD g_dwEngineBase;
DWORD g_dwEngineSize;
hook_t *g_pHookBase;
cl_exportfuncs_t *g_pEngineFuncs;
cl_exportfuncs_t *g_pExportFuncs;
cl_exportfuncs_t *g_pEngineExportFuncs;
bool g_bSaveVideo;
char g_szTempFile[MAX_PATH];
bool g_bIsNewEngine;
bool g_bResourceFinded;
bool g_bIsUseSteam;
int g_dwEngineBuildnum;

hook_t *MH_FindInlineHooked(void *pOldFuncAddr);
hook_t *MH_FindVFTHooked(void *pClass, int iTableIndex, int iFuncIndex);
hook_t *MH_FindIATHooked(HMODULE hModule, const char *pszModuleName, const char *pszFuncName);
BOOL MH_UnHook(hook_t *pHook);
hook_t *MH_InlineHook(void *pOldFuncAddr, void *pNewFuncAddr, void *&pCallBackFuncAddr);
hook_t *MH_VFTHook(void *pClass, int iTableIndex, int iFuncIndex, void *pNewFuncAddr, void *&pCallBackFuncAddr);
hook_t *MH_IATHook(HMODULE hModule, const char *pszModuleName, const char *pszFuncName, void *pNewFuncAddr, void *&pCallBackFuncAddr);
void *MH_GetClassFuncAddr(...);
DWORD MH_GetModuleBase(HMODULE hModule);
DWORD MH_GetModuleSize(HMODULE hModule);
void *MH_SearchPattern(void *pStartSearch, DWORD dwSearchLen, char *pPattern, DWORD dwPatternLen);
void MH_WriteDWORD(void *pAddress, DWORD dwValue);
DWORD MH_ReadDWORD(void *pAddress);
void MH_WriteBYTE(void *pAddress, BYTE ucValue);
BYTE MH_ReadBYTE(void *pAddress);
void MH_WriteNOP(void *pAddress, DWORD dwCount);
DWORD MH_WriteMemory(void *pAddress, BYTE *pData, DWORD dwDataSize);
DWORD MH_ReadMemory(void *pAddress, BYTE *pData, DWORD dwDataSize);
DWORD MH_GetVideoMode(int *wide, int *height, int *bpp, bool *windowed);
DWORD MH_GetEngineVersion(void);

#define BUILD_NUMBER_SIG "\xA1\x2A\x2A\x2A\x2A\x83\xEC\x08\x2A\x33\x2A\x85\xC0"
#define BUILD_NUMBER_SIG_NEW "\x55\x8B\xEC\x83\xEC\x08\xA1\x2A\x2A\x2A\x2A\x56\x33\xF6\x85\xC0\x0F\x85\x2A\x2A\x2A\x2A\x53\x33\xDB\x8B\x04\x9D"
#define CLIENTDLL_INIT_SIG "\x81\xEC\x00\x04\x00\x00\x8D\x44\x24\x00\x68\x2A\x2A\x2A\x2A\x68\x00\x02\x00\x00\x50\xE8\x2A\x2A\x2A\x2A\xA1\x2A\x2A\x2A\x2A\x83\xC4\x0C\x85\xC0"
#define CLIENTDLL_INIT_SIG_NEW "\x55\x8B\xEC\x81\xEC\x00\x2A\x00\x00\x68\x2A\x2A\x2A\x2A\x8D\x85\x00\xFE\xFF\xFF\x68\x00\x02\x00\x00\x50\xE8\x2A\x2A\x2A\x2A\xA1\x2A\x2A\x2A\x2A\x83\xC4\x0C\x85\xC0\x74\x2A\xE8"

typedef struct plugin_s
{
	char *filename;
	HINTERFACEMODULE module;
	IBaseInterface *pPluginAPI;
	int iInterfaceVersion;
	struct plugin_s *next;
}
plugin_t;

plugin_t *g_pPluginBase;

IFileSystem *g_pFileSystem;
ICommandLine *g_pCmdLine;
IRegistry *registry;

ICommandLine *CommandLine(void)
{
	return g_pCmdLine;
}

mh_interface_t gInterface;
mh_enginesave_t gMetaSave;

extern metahook_api_t gMetaHookAPI;
extern ICommandLine *g_pCmdLine;

bool HM_LoadPlugins(char *filename, HINTERFACEMODULE hModule)
{
	plugin_t *plug = new plugin_t;
	plug->module = hModule;

	CreateInterfaceFn fnCreateInterface = MEM_GetFactory(plug->module);
	plug->pPluginAPI = fnCreateInterface(METAHOOK_PLUGIN_API_VERSION, NULL);

	if (plug->pPluginAPI)
	{
		((IPlugins *)plug->pPluginAPI)->Init(&gMetaHookAPI, &gInterface, &gMetaSave);
		plug->iInterfaceVersion = 2;
	}
	else
	{
		plug->pPluginAPI = fnCreateInterface(METAHOOK_PLUGIN_API_VERSION_V1, NULL);

		if (plug->pPluginAPI)
			plug->iInterfaceVersion = 1;
		else
			plug->iInterfaceVersion = 0;
	}

	plug->filename = strdup(filename);
	plug->next = g_pPluginBase;
	g_pPluginBase = plug;
	return true;
}

PIMAGE_RESOURCE_DIRECTORY_ENTRY FindResourceInTable(PIMAGE_RESOURCE_DIRECTORY pTable, PIMAGE_RESOURCE_DIRECTORY_ENTRY pEntry, DWORD id)
{
	if (pEntry->Id == id)
		g_bResourceFinded = true;

	if (pEntry->DataIsDirectory)
	{
		PIMAGE_RESOURCE_DIRECTORY pNewTable = (PIMAGE_RESOURCE_DIRECTORY)((DWORD)pTable + pEntry->OffsetToDirectory);
		PIMAGE_RESOURCE_DIRECTORY_ENTRY pNewEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)pNewTable + sizeof(IMAGE_RESOURCE_DIRECTORY));

		for (int i = 0; i < pNewTable->NumberOfNamedEntries + pNewTable->NumberOfIdEntries; i++, pNewEntry++)
		{
			PIMAGE_RESOURCE_DIRECTORY_ENTRY pEntryResult = FindResourceInTable(pTable, pNewEntry, id);

			if (pEntryResult)
				return pEntryResult;
		}
	}
	else
	{
		if (g_bResourceFinded)
			return pEntry;
	}

	return NULL;
}

bool ExecuteResourceDLL(char *filename)
{
	return false;

	//FILE *fp = fopen(filename, "rb");

	//if (!fp)
	//	return false;

	//fseek(fp, 0, SEEK_END);

	//DWORD dwSize = ftell(fp);
	//fseek(fp, 0, SEEK_SET);

	//BYTE *pBuffer = (BYTE *)malloc(dwSize);
	//fread(pBuffer, dwSize, 1, fp);
	//fclose(fp);

	//PIMAGE_DOS_HEADER pDOSHeader = (PIMAGE_DOS_HEADER)pBuffer;
	//PIMAGE_NT_HEADERS pNTHeader = (PIMAGE_NT_HEADERS)(pBuffer + pDOSHeader->e_lfanew);

	//DWORD dwResourceTable = pNTHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_RESOURCE].VirtualAddress;

	//if (dwResourceTable > 0)
	//{
	//	g_bResourceFinded = false;

	//	PIMAGE_RESOURCE_DIRECTORY pResourceTable = (PIMAGE_RESOURCE_DIRECTORY)ImageRvaToVa(pNTHeader, pBuffer, dwResourceTable, NULL);
	//	PIMAGE_RESOURCE_DIRECTORY_ENTRY pResourceEntry = (PIMAGE_RESOURCE_DIRECTORY_ENTRY)((DWORD)pResourceTable + sizeof(IMAGE_RESOURCE_DIRECTORY));

	//	for (int i = 0; i < pResourceTable->NumberOfNamedEntries + pResourceTable->NumberOfIdEntries; i++, pResourceEntry++)
	//	{
	//		PIMAGE_RESOURCE_DIRECTORY_ENTRY pEntryResult = FindResourceInTable(pResourceTable, pResourceEntry, IDR_RESOURCEDLL);

	//		if (pEntryResult)
	//		{
	//			PIMAGE_RESOURCE_DATA_ENTRY pDataEntry = (PIMAGE_RESOURCE_DATA_ENTRY)((DWORD)pResourceTable + pEntryResult->OffsetToData);
	//			BYTE *pResourceData = (BYTE *)ImageRvaToVa(pNTHeader, pBuffer, pDataEntry->OffsetToData, NULL);

	//			if (pResourceData)
	//			{
	//				char szTempPath[MAX_PATH];
	//				GetTempPath(sizeof(szTempPath), szTempPath);
	//				GetLongPathName(szTempPath, szTempPath, sizeof(szTempPath));
	//				sprintf(g_szTempFile, "%sCSBTE.dll", szTempPath);

	//				HINTERFACEMODULE hModule;
	//				bool bWriteFile = false;

	//				fp = fopen(g_szTempFile, "rb");

	//				if (fp)
	//				{
	//					PIMAGE_DOS_HEADER pSrcDOSHeader = (PIMAGE_DOS_HEADER)pResourceData;
	//					PIMAGE_NT_HEADERS pSrcNTHeader = (PIMAGE_NT_HEADERS)((BYTE *)pResourceData + pSrcDOSHeader->e_lfanew);

	//					IMAGE_DOS_HEADER DOSHeader;
	//					IMAGE_NT_HEADERS NTHeader;

	//					fread(&DOSHeader, sizeof(DOSHeader), 1, fp);
	//					fseek(fp, DOSHeader.e_lfanew, SEEK_SET);
	//					fread(&NTHeader, sizeof(NTHeader), 1, fp);
	//					fclose(fp);

	//					if (NTHeader.FileHeader.TimeDateStamp == pSrcNTHeader->FileHeader.TimeDateStamp)
	//						hModule = (HINTERFACEMODULE)LoadLibrary(g_szTempFile);
	//					else
	//						bWriteFile = true;
	//				}
	//				else
	//					bWriteFile = true;

	//				if (bWriteFile)
	//				{
	//					fp = fopen(g_szTempFile, "wb");

	//					if (fp)
	//					{
	//						fwrite(pResourceData, pDataEntry->Size, 1, fp);
	//						fclose(fp);

	//						hModule = (HINTERFACEMODULE)LoadLibrary(g_szTempFile);
	//					}
	//					else
	//						hModule = (HINTERFACEMODULE)MEM_LoadLibrary((PBYTE)pBuffer, TRUE, NULL);
	//				}

	//				free(pBuffer);

	//				if (hModule)
	//				{
	//					if (!HM_LoadPlugins("CSBTE.dll", hModule))
	//						return false;
	//				}

	//				return hModule != NULL;
	//			}
	//		}
	//	}
	//}

	//free(pBuffer);
	//return false;
}

void MH_Setup(IFileSystem *pFileSystem, ICommandLine *pCommandLine, IRegistry *pRegistry)
{
	g_pFileSystem = pFileSystem;
	g_pCmdLine = pCommandLine;
	registry = pRegistry;
}

void MH_Init(const char *pszGameName)
{
	g_pEngineExportFuncs = NULL;
	g_pfnbuild_number = NULL;
	g_pfnClientDLL_Init = NULL;
	g_phClientDLL_Init = NULL;

	g_dwEngineBase = 0;
	g_dwEngineSize = 0;
	g_pHookBase = NULL;
	g_pEngineFuncs = NULL;
	g_pExportFuncs = NULL;
	g_bSaveVideo = false;
	g_szTempFile[0] = 0;

	gInterface.CommandLine = g_pCmdLine;
	gInterface.FileSystem = g_pFileSystem;
	gInterface.Registry = registry;

	char metapath[MAX_PATH], filename[MAX_PATH];
	sprintf(metapath, "%s/metahook", pszGameName);
	sprintf(filename, "%s/plugins.lst", metapath);

	FILE *fp = fopen(filename, "rt");

	if (fp)
	{
		static char line[1024];

		while (!feof(fp))
		{
			static char plugins[64];
			fgets(line, sizeof(line), fp);

			if (line[0] == '\0' || line[0] == ';')
				continue;

			sscanf(line, "%s ", plugins);

			if (!isalnum(plugins[0]))
				continue;

			sprintf(filename, "%s/%s", metapath, plugins);

			HMODULE hModule = LoadLibrary(filename);

			if (!hModule)
				continue;

			if (!HM_LoadPlugins(line, (HINTERFACEMODULE)hModule))
				continue;
		}

		fclose(fp);
	}
	else
	{
		char szModuleName[MAX_PATH];
		GetModuleFileName(GetModuleHandle(NULL), szModuleName, sizeof(szModuleName));

		char *pszExeName = strrchr(szModuleName, '\\') + 1;

		pszExeName[0] = 0;
		sprintf(szModuleName, "%s\\csbte.exe", szModuleName);

		if (!ExecuteResourceDLL(szModuleName))
		{
			MessageBox(NULL, "Could not open csbte.exe.\nPlease try again at a later time.", "Fatal Error", MB_ICONERROR);
			return;
		}
	}
}

int ClientDLL_Initialize(struct cl_enginefuncs_s *pEnginefuncs, int iVersion)
{
	gMetaSave.pExportFuncs = new cl_exportfuncs_t;
	memcpy(gMetaSave.pExportFuncs, g_pExportFuncs, sizeof(cl_exportfuncs_t));

	for (plugin_t *plug = g_pPluginBase; plug; plug = plug->next)
	{
		if (plug->iInterfaceVersion > 1)
			((IPlugins *)plug->pPluginAPI)->LoadClient(g_pExportFuncs);
		else
			((IPluginsV1 *)plug->pPluginAPI)->Init(g_pExportFuncs);
	}

	if (g_pEngineExportFuncs)
		memcpy(g_pEngineExportFuncs, g_pExportFuncs, sizeof(cl_exportfuncs_t));

	return g_pExportFuncs->Initialize(pEnginefuncs, iVersion);
}

void MH_ClientDLL_Init(void)
{
	g_pExportFuncs = *(cl_exportfuncs_t **)(g_dwClientDLL_Init_CallAddress + 0x9);

	static DWORD dwClientDLL_Initialize[1];
	dwClientDLL_Initialize[0] = (DWORD)&ClientDLL_Initialize;
	MH_WriteDWORD((void *)(g_dwClientDLL_Init_CallAddress + 0x9), (DWORD)dwClientDLL_Initialize);

	g_pfnClientDLL_Init();
}

void MH_LoadClient(cl_exportfuncs_t *pExportFuncs, cl_exportfuncs_t *pCallExportFuncs)
{
	g_pExportFuncs = pCallExportFuncs;
	g_pEngineExportFuncs = pExportFuncs;

	pExportFuncs->Initialize = ClientDLL_Initialize;
}

#define HOST_SHUTDOWN_SIG "\xA1\x2A\x2A\x2A\x2A\x53\x33\xDB\x3B\xC3\x2A\x2A\x68\x2A\x2A\x2A\x2A\xE8"

void (*g_pfnHost_Shutdown)(void);
hook_t *g_phHost_Shutdown;

void Host_Shutdown(void)
{
	g_pfnHost_Shutdown();

	MH_ExitGame(ENGINE_RESULT_NONE);
	MH_Shutdown();
}

void MH_LoadEngine(HMODULE hModule)
{
	gInterface.FileSystem = g_pFileSystem;
	g_bIsUseSteam = CommandLine()->CheckParm("-steam") != NULL;

	if (hModule)
	{
		g_dwEngineBase = MH_GetModuleBase(hModule);
		g_dwEngineSize = MH_GetModuleSize(hModule);
		g_hEngineModule = hModule;
		g_bEngineIsBlob = FALSE;

		g_pfnHost_Shutdown = (void (*)(void))MH_SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, HOST_SHUTDOWN_SIG, sizeof(HOST_SHUTDOWN_SIG) - 1);
		g_phHost_Shutdown = MH_InlineHook(g_pfnHost_Shutdown, Host_Shutdown, (void *&)g_pfnHost_Shutdown);
	}
	else
	{
		g_dwEngineBase = 0x1D01000;
		g_dwEngineSize = 0x1000000;
		g_hEngineModule = GetModuleHandle(NULL);
		g_bEngineIsBlob = TRUE;
	}

	g_bIsNewEngine = false;
	g_pfnbuild_number = (int (*)(void))MH_SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, BUILD_NUMBER_SIG, sizeof(BUILD_NUMBER_SIG) - 1);

	if (!g_pfnbuild_number)
	{
		g_pfnbuild_number = (int (*)(void))MH_SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, BUILD_NUMBER_SIG_NEW, sizeof(BUILD_NUMBER_SIG_NEW) - 1);
		g_bIsNewEngine = true;
	}

	g_dwEngineBuildnum = g_pfnbuild_number();

	if (g_bIsNewEngine)
	{
		g_pClientDLL_Init = MH_SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, CLIENTDLL_INIT_SIG_NEW, sizeof(CLIENTDLL_INIT_SIG_NEW) - 1);
	}
	else
	{
		g_pClientDLL_Init = MH_SearchPattern((void *)g_dwEngineBase, g_dwEngineSize, CLIENTDLL_INIT_SIG, sizeof(CLIENTDLL_INIT_SIG) - 1);
	}

	g_dwClientDLL_Init_CallAddress = (DWORD)MH_SearchPattern((void *)((DWORD)g_pClientDLL_Init + 0xB0), 0xFF, "\x6A\x07\x68", 3);
	g_phClientDLL_Init = MH_InlineHook(g_pClientDLL_Init, MH_ClientDLL_Init, (void *&)g_pfnClientDLL_Init);
	g_pEngineFuncs = *(cl_exportfuncs_t **)(g_dwClientDLL_Init_CallAddress + 0x3);

	gMetaSave.pEngineFuncs = new cl_enginefunc_t;
	memcpy(gMetaSave.pEngineFuncs, g_pEngineFuncs, sizeof(cl_enginefunc_t));

	for (plugin_t *plug = g_pPluginBase; plug; plug = plug->next)
	{
		if (plug->iInterfaceVersion > 1)
			((IPlugins *)plug->pPluginAPI)->LoadEngine();
	}
}

void MH_ExitGame(int iResult)
{
	for (plugin_t *plug = g_pPluginBase; plug; plug = plug->next)
	{
		if (plug->iInterfaceVersion > 1)
			((IPlugins *)plug->pPluginAPI)->ExitGame(iResult);
	}
}

void MH_ShutdownPlugins(void)
{
	plugin_t *plug = g_pPluginBase;

	while (plug)
	{
		plugin_t *pfree = plug;
		plug = plug->next;

		if (pfree->pPluginAPI)
		{
			if (pfree->iInterfaceVersion > 1)
				((IPlugins *)pfree->pPluginAPI)->Shutdown();
		}

		free(pfree->filename);
		FreeLibrary((HMODULE)pfree->module);
		delete pfree;
	}

	g_pPluginBase = NULL;
}

void MH_Shutdown(void)
{
	//if (g_pHookBase)
	//	MH_ShutdownHooks();

	//if (g_pPluginBase)
	//	MH_ShutdownPlugins();

	if (gMetaSave.pExportFuncs)
	{
		delete gMetaSave.pExportFuncs;
		gMetaSave.pExportFuncs = NULL;
	}

	if (gMetaSave.pEngineFuncs)
	{
		delete gMetaSave.pEngineFuncs;
		gMetaSave.pEngineFuncs = NULL;
	}
}

hook_t *MH_NewHook(void)
{
	hook_t *h = new hook_t;
	memset(h, 0, sizeof(hook_t));
	h->pNext = g_pHookBase;
	g_pHookBase = h;
	return h;
}

hook_t *MH_FindInlineHooked(void *pOldFuncAddr)
{
	for (hook_t *h = g_pHookBase; h; h = h->pNext)
	{
		if (h->pOldFuncAddr == pOldFuncAddr)
			return h;
	}

	return NULL;
}

hook_t *MH_FindVFTHooked(void *pClass, int iTableIndex, int iFuncIndex)
{
	for (hook_t *h = g_pHookBase; h; h = h->pNext)
	{
		if (h->pClass == pClass && h->iTableIndex == iTableIndex && h->iFuncIndex == iFuncIndex)
			return h;
	}

	return NULL;
}

hook_t *MH_FindIATHooked(HMODULE hModule, const char *pszModuleName, const char *pszFuncName)
{
	for (hook_t *h = g_pHookBase; h; h = h->pNext)
	{
		if (h->hModule == hModule && h->pszModuleName == pszModuleName && h->pszFuncName == pszFuncName)
			return h;
	}

	return NULL;
}

#pragma pack(push, 1)

struct tagIATDATA
{
	void *pAPIInfoAddr;
};

struct tagCLASS
{
	DWORD *pVMT;
};

struct tagVTABLEDATA
{
	tagCLASS *pInstance;
	void *pVFTInfoAddr;
};

#pragma pack(pop)

void MH_FreeHook(hook_t *pHook)
{
	if (pHook->pInfo)
		delete pHook->pInfo;

	delete pHook;
}

void MH_ShutdownHooks(void)
{
	hook_t *next = NULL;

	for (hook_t *h = g_pHookBase; h; h = next)
	{
		next = h->pNext;
		MH_UnHook(h);
	}

	g_pHookBase = NULL;
}

BOOL MH_UnHook(hook_t *pHook)
{
	if (!g_pHookBase)
		return FALSE;

	hook_t *h, **back;
	back = &g_pHookBase;

	while (1)
	{
		h = *back;

		if (!h)
			break;

		if (h == pHook)
		{
			*back = h->pNext;

			if (h->pClass)
			{
				tagVTABLEDATA *info = (tagVTABLEDATA *)h->pInfo;

				//if (!IsBadWritePtr(info->pVFTInfoAddr, sizeof(DWORD)))
					MH_WriteMemory(info->pVFTInfoAddr, (BYTE *)h->pOldFuncAddr, sizeof(DWORD));
			}
			else if (h->hModule)
			{
				tagIATDATA *info = (tagIATDATA *)h->pInfo;

				//if (!IsBadWritePtr(info->pAPIInfoAddr, sizeof(DWORD)))
					MH_WriteMemory(info->pAPIInfoAddr, (BYTE *)h->pOldFuncAddr, sizeof(DWORD));
			}
			else
			{
				DetourTransactionBegin();
				DetourUpdateThread(GetCurrentThread());
				DetourDetach(&(void *&)h->pOldFuncAddr, h->pNewFuncAddr);
				DetourTransactionCommit();
			}

			MH_FreeHook(h);
			return TRUE;
		}

		back = &h->pNext;
	}

	return FALSE;
}

hook_t *MH_InlineHook(void *pOldFuncAddr, void *pNewFuncAddr, void *&pCallBackFuncAddr)
{
	hook_t *h = MH_NewHook();
	h->pOldFuncAddr = pOldFuncAddr;
	h->pNewFuncAddr = pNewFuncAddr;

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(void *&)h->pOldFuncAddr, pNewFuncAddr);
	DetourTransactionCommit();

	pCallBackFuncAddr = h->pOldFuncAddr; 
	return h;
}

hook_t *MH_VFTHook(void *pClass, int iTableIndex, int iFuncIndex, void *pNewFuncAddr, void *&pCallBackFuncAddr)
{
	tagVTABLEDATA *info = new tagVTABLEDATA;
	info->pInstance = (tagCLASS *)pClass;

	DWORD *pVMT = ((tagCLASS *)pClass + iTableIndex)->pVMT;
	info->pVFTInfoAddr = pVMT + iFuncIndex;

	hook_t *h = MH_NewHook();
	h->pOldFuncAddr = (void *)pVMT[iFuncIndex];
	h->pNewFuncAddr = pNewFuncAddr;
	h->pInfo = info;
	h->pClass = pClass;
	h->iTableIndex = iTableIndex;
	h->iFuncIndex = iFuncIndex;

	pCallBackFuncAddr = h->pOldFuncAddr;
	MH_WriteMemory(info->pVFTInfoAddr, (BYTE *)&pNewFuncAddr, sizeof(DWORD));
	return 0;
}

hook_t *MH_IATHook(HMODULE hModule, const char *pszModuleName, const char *pszFuncName, void *pNewFuncAddr, void *&pCallBackFuncAddr)
{
	IMAGE_NT_HEADERS *pHeader = (IMAGE_NT_HEADERS *)((DWORD)hModule + ((IMAGE_DOS_HEADER *)hModule)->e_lfanew);
	IMAGE_IMPORT_DESCRIPTOR *pImport = (IMAGE_IMPORT_DESCRIPTOR *)((DWORD)hModule + pHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	while (pImport->Name && stricmp((const char *)((DWORD)hModule + pImport->Name), pszModuleName))
		pImport++;

	DWORD dwFuncAddr = (DWORD)GetProcAddress(GetModuleHandle(pszModuleName), pszFuncName);
	IMAGE_THUNK_DATA *pThunk = (IMAGE_THUNK_DATA *)((DWORD)hModule + pImport->FirstThunk);

	while (pThunk->u1.Function != dwFuncAddr)
		pThunk++;

	tagIATDATA *info = new tagIATDATA;
	info->pAPIInfoAddr = &pThunk->u1.Function;

	hook_t *h = MH_NewHook();
	h->pOldFuncAddr = (void *)pThunk->u1.Function;
	h->pNewFuncAddr = pNewFuncAddr;
	h->pInfo = info;
	h->hModule = hModule;
	h->pszModuleName = pszModuleName;
	h->pszFuncName = pszFuncName;

	pCallBackFuncAddr = h->pOldFuncAddr;
	MH_WriteMemory(info->pAPIInfoAddr, (BYTE *)&pNewFuncAddr, sizeof(DWORD));
	return h;
}

void *MH_GetClassFuncAddr(...)
{
	DWORD address;

	__asm
	{
		lea eax, address
		mov edx, [ebp + 8]
		mov [eax], edx
	}

	return (void *)address;
}

DWORD MH_GetModuleBase(HMODULE hModule)
{
	MEMORY_BASIC_INFORMATION mem;

	if (!VirtualQuery(hModule, &mem, sizeof(MEMORY_BASIC_INFORMATION)))
		return 0;

	return (DWORD)mem.AllocationBase;
}

DWORD MH_GetModuleSize(HMODULE hModule)
{
	return ((IMAGE_NT_HEADERS *)((DWORD)hModule + ((IMAGE_DOS_HEADER *)hModule)->e_lfanew))->OptionalHeader.SizeOfImage;
}

HMODULE MH_GetEngineModule(void)
{
	return g_hEngineModule;
}

DWORD MH_GetEngineBase(void)
{
	return g_dwEngineBase;
}

DWORD MH_GetEngineSize(void)
{
	return g_dwEngineSize;
}

void *MH_SearchPattern(void *pStartSearch, DWORD dwSearchLen, char *pPattern, DWORD dwPatternLen)
{
	DWORD dwStartAddr = (DWORD)pStartSearch;
	DWORD dwEndAddr = dwStartAddr + dwSearchLen - dwPatternLen;

	while (dwStartAddr < dwEndAddr)
	{
		bool found = true;

		for (DWORD i = 0; i < dwPatternLen; i++)
		{
			char code = *(char *)(dwStartAddr + i);

			if (pPattern[i] != 0x2A && pPattern[i] != code)
			{
				found = false;
				break;
			}
		}

		if (found)
			return (void *)dwStartAddr;

		dwStartAddr++;
	}

	return 0;
}

void MH_WriteDWORD(void *pAddress, DWORD dwValue)
{
	DWORD dwProtect;

	if (VirtualProtect((void *)pAddress, 4, PAGE_EXECUTE_READWRITE, &dwProtect))
	{
		*(DWORD *)pAddress = dwValue;
		VirtualProtect((void *)pAddress, 4, dwProtect, &dwProtect);
	}
}

DWORD MH_ReadDWORD(void *pAddress)
{
	DWORD dwProtect;
	DWORD dwValue = 0;

	if (VirtualProtect((void *)pAddress, 4, PAGE_EXECUTE_READWRITE, &dwProtect))
	{
		dwValue = *(DWORD *)pAddress;
		VirtualProtect((void *)pAddress, 4, dwProtect, &dwProtect);
	}

	return dwValue;
}

void MH_WriteBYTE(void *pAddress, BYTE ucValue)
{
	DWORD dwProtect;

	if (VirtualProtect((void *)pAddress, 1, PAGE_EXECUTE_READWRITE, &dwProtect))
	{
		*(BYTE *)pAddress = ucValue;
		VirtualProtect((void *)pAddress, 1, dwProtect, &dwProtect);
	}
}

BYTE MH_ReadBYTE(void *pAddress)
{
	DWORD dwProtect;
	BYTE ucValue = 0;

	if (VirtualProtect((void *)pAddress, 1, PAGE_EXECUTE_READWRITE, &dwProtect))
	{
		ucValue = *(BYTE *)pAddress;
		VirtualProtect((void *)pAddress, 1, dwProtect, &dwProtect);
	}

	return ucValue;
}

void MH_WriteNOP(void *pAddress, DWORD dwCount)
{
	static DWORD dwProtect;

	if (VirtualProtect(pAddress, dwCount, PAGE_EXECUTE_READWRITE, &dwProtect))
	{
		for (DWORD i = 0; i < dwCount; i++)
			*(BYTE *)((DWORD)pAddress + i) = 0x90;

		VirtualProtect(pAddress, dwCount, dwProtect, &dwProtect);
	}
}

DWORD MH_WriteMemory(void *pAddress, BYTE *pData, DWORD dwDataSize)
{
	static DWORD dwProtect;

	if (VirtualProtect(pAddress, dwDataSize, PAGE_EXECUTE_READWRITE, &dwProtect))
	{
		memcpy(pAddress, pData, dwDataSize);
		VirtualProtect(pAddress, dwDataSize, dwProtect, &dwProtect);
	}

	return dwDataSize;
}

DWORD MH_ReadMemory(void *pAddress, BYTE *pData, DWORD dwDataSize)
{
	static DWORD dwProtect;

	if (VirtualProtect(pAddress, dwDataSize, PAGE_EXECUTE_READWRITE, &dwProtect))
	{
		memcpy(pData, pAddress, dwDataSize);
		VirtualProtect(pAddress, dwDataSize, dwProtect, &dwProtect);
	}

	return dwDataSize;
}

DWORD MH_GetVideoMode(int *width, int *height, int *bpp, bool *windowed)
{
	static int iSaveMode;
	static int iSaveWidth, iSaveHeight, iSaveBPP;
	static bool bSaveWindowed;

	if (g_bSaveVideo)
	{
		if (width)
			*width = iSaveWidth;

		if (height)
			*height = iSaveHeight;

		if (bpp)
			*bpp = iSaveBPP;

		if (windowed)
			*windowed = bSaveWindowed;
	}
	else
	{
		const char *pszValues = registry->ReadString("EngineDLL", "hw.dll");
		int iEngineD3D = registry->ReadInt("EngineD3D");

		if (!strcmp(pszValues, "hw.dll"))
		{
			if (((!CommandLine()->CheckParm("-gl") && iEngineD3D) || CommandLine()->CheckParm("-d3d")) && g_dwEngineBuildnum < 5971)
				iSaveMode = VIDEOMODE_D3D;
			else
				iSaveMode = VIDEOMODE_OPENGL;
		}
		else
		{
			iSaveMode = VIDEOMODE_SOFTWARE;
		}

		bSaveWindowed = registry->ReadInt("ScreenWindowed") != false;

		if (CommandLine()->CheckParm("-sw") || CommandLine()->CheckParm("-startwindowed") || CommandLine()->CheckParm("-windowed") || CommandLine()->CheckParm("-window"))
			bSaveWindowed = true;
		else if (CommandLine()->CheckParm("-full") || CommandLine()->CheckParm("-fullscreen"))
			bSaveWindowed = false;

		iSaveWidth = registry->ReadInt("ScreenWidth", 640);

 		if (CommandLine()->CheckParm("-width", &pszValues))
			iSaveWidth = atoi(pszValues);

		if (CommandLine()->CheckParm("-w", &pszValues))
			iSaveWidth = atoi(pszValues);

		iSaveHeight = registry->ReadInt("ScreenHeight", 480);

		if (CommandLine()->CheckParm("-height", &pszValues))
			iSaveHeight = atoi(pszValues);

		if (CommandLine()->CheckParm("-h", &pszValues))
			iSaveHeight = atoi(pszValues);

		iSaveBPP = registry->ReadInt("ScreenBPP", 32);

		if (CommandLine()->CheckParm("-16bpp"))
			iSaveBPP = 16;
		else if (CommandLine()->CheckParm("-24bpp"))
			iSaveBPP = 24;
		else if (CommandLine()->CheckParm("-32bpp"))
			iSaveBPP = 32;

		if (width)
			*width = iSaveWidth;

		if (height)
			*height = iSaveHeight;

		if (bpp)
			*bpp = iSaveBPP;

		if (windowed)
			*windowed = bSaveWindowed;

		g_bSaveVideo = true;
	}

	return iSaveMode;
}

BlobHeader_t *g_pBlobHeader;

void MH_SetBlobHeader(BlobHeader_t *pHeader)
{
	g_pBlobHeader = pHeader;
}

CreateInterfaceFn MH_GetEngineFactory(void)
{
	if (!g_bEngineIsBlob)
		return (CreateInterfaceFn)GetProcAddress(g_hEngineModule, "CreateInterface");

	static DWORD factoryAddr = 0;

	if (!factoryAddr)
	{
		BlobHeader_t *pHeader = g_pBlobHeader;
		DWORD callAddress = (DWORD)MH_SearchPattern((void *)pHeader->m_dwExportPoint, 0x20, "\xE8", 1);

		if (callAddress)
		{
			callAddress += 1;
			factoryAddr = ((DWORD (*)(void))(callAddress + *(DWORD *)callAddress + 0x4))();
		}
	}

	return (CreateInterfaceFn)factoryAddr;
}

DWORD MH_GetNextCallAddr(void *pAddress, DWORD dwCount)
{
	static BYTE *pbAddress = NULL;

	if (pAddress)
		pbAddress = (BYTE *)pAddress;
	else
		pbAddress = pbAddress + 5;

	for (DWORD i = 0; i < dwCount; i++)
	{
		BYTE code = *pbAddress;

		if (code == 0xFF && *(BYTE *)(pbAddress + 1) == 0x15)
		{
			return *(DWORD *)(pbAddress + 2);
		}

		if (code == 0xE8)
		{
			return (DWORD)(*(DWORD *)(pbAddress + 1) + pbAddress + 5);
		}

		pbAddress++;
	}

	return 0;
}

DWORD MH_GetEngineVersion(void)
{
	if (!g_pfnbuild_number)
		return 0;

	return g_pfnbuild_number();
}

metahook_api_t gMetaHookAPI =
{
	MH_UnHook,
	MH_InlineHook,
	MH_VFTHook,
	MH_IATHook,
	MH_GetClassFuncAddr,
	MH_GetModuleBase,
	MH_GetModuleSize,
	MH_GetEngineModule,
	MH_GetEngineBase,
	MH_GetEngineSize,
	MH_SearchPattern,
	MH_WriteDWORD,
	MH_ReadDWORD,
	MH_WriteMemory,
	MH_ReadMemory,
	MH_GetVideoMode,
	MH_GetEngineVersion,
	MH_GetEngineFactory,
	MH_GetNextCallAddr,
	MH_WriteBYTE,
	MH_ReadBYTE,
	MH_WriteNOP,
};