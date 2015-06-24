#include <metahook.h>

HMODULE g_hMetaHook = NULL;

void MH_LoadLibrary(const char *pszDLLPath)
{
	g_hMetaHook = LoadLibrary(pszDLLPath);
}

void MH_FreeLibrary(void)
{
	FreeLibrary(g_hMetaHook);
}

void MH_Setup(IFileSystem *pFileSystem, ICommandLine *pCommandLine, IRegistry *pRegistry)
{
	if (!g_hMetaHook)
		return;

	static void (*MH_Setup)(IFileSystem *, ICommandLine *, IRegistry *) = (void (*)(IFileSystem *, ICommandLine *, IRegistry *))GetProcAddress(g_hMetaHook, "MH_Setup");

	if (MH_Setup)
		MH_Setup(pFileSystem, pCommandLine, pRegistry);
}

void MH_Init(const char *pszGameName)
{
	if (!g_hMetaHook)
		return;

	static void (*MH_Init)(const char *pszGameName) = (void (*)(const char *))GetProcAddress(g_hMetaHook, "MH_Init");

	if (MH_Init)
		MH_Init(pszGameName);
}

void MH_SetBlobHeader(struct BlobHeader_s *pHeader)
{
	if (!g_hMetaHook)
		return;

	void (*MH_SetBlobHeader)(struct BlobHeader_s *pHeader) = (void (*)(struct BlobHeader_s *))GetProcAddress(g_hMetaHook, "MH_SetBlobHeader");

	if (MH_SetBlobHeader)
		MH_SetBlobHeader(pHeader);
}

void MH_LoadClient(cl_exportfuncs_t *pExportFuncs, cl_exportfuncs_t *pCallExportFuncs)
{
	if (!g_hMetaHook)
		return;

	void (*MH_LoadClient)(cl_exportfuncs_t *pExportFuncs, cl_exportfuncs_t *pCallExportFuncs) = (void (*)(cl_exportfuncs_t *, cl_exportfuncs_t *))GetProcAddress(g_hMetaHook, "MH_LoadClient");

	if (MH_LoadClient)
		MH_LoadClient(pExportFuncs, pCallExportFuncs);
}

void MH_LoadEngine(HMODULE hModule)
{
	if (!g_hMetaHook)
		return;

	void (*MH_LoadEngine)(HMODULE hModule) = (void (*)(HMODULE))GetProcAddress(g_hMetaHook, "MH_LoadEngine");

	if (MH_LoadEngine)
		MH_LoadEngine(hModule);
}

void MH_ExitGame(int iResult)
{
	if (!g_hMetaHook)
		return;

	void (*MH_ExitGame)(int iResult) = (void (*)(int))GetProcAddress(g_hMetaHook, "MH_ExitGame");

	if (MH_ExitGame)
		MH_ExitGame(iResult);
}

void MH_Shutdown(void)
{
	if (!g_hMetaHook)
		return;

	void (*MH_Shutdown)(void) = (void (*)(void))GetProcAddress(g_hMetaHook, "MH_Shutdown");

	if (MH_Shutdown)
		MH_Shutdown();
}