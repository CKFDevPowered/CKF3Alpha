#include <metahook.h>
#include <IEngine.h>
#include "LoadBlob.h"

HMODULE g_hThisModule;
HMODULE g_hFileSystemModule;
HMODULE g_hEngineModule = NULL;
HMODULE g_hClientModule;

bool g_bIsUseSteam;
bool g_bMetaHook;
BlobFootprint_t g_blobfootprintClient;
char g_szModulePath[MAX_PATH];

cl_enginefunc_t gEngfuncs;
cl_exportfuncs_t gExportfuncs;
cl_exportfuncs_t *gpExportfuncs;
modfuncs_t g_modfuncs;
cldll_func_dst_t g_cldstAddrs;

IFileSystem *g_pFileSystem;

void EmptyFunction(void)
{
	return;
}

void GetModulePath(void)
{
	GetModuleFileName(g_hThisModule, g_szModulePath, sizeof(g_szModulePath));

	char *pszResult = strrchr(g_szModulePath, '\\');

	if (pszResult)
		*pszResult = '\0';
}

void GetFileSystem(void)
{
	g_hFileSystemModule = GetModuleHandle("FileSystem_Stdio.dll");

	if (!g_hFileSystemModule)
		g_hFileSystemModule = GetModuleHandle("FileSystem_Steam.dll");

	CreateInterfaceFn fnCreateInterface = (CreateInterfaceFn)GetProcAddress(g_hFileSystemModule, CREATEINTERFACE_PROCNAME);

	if (fnCreateInterface)
		g_pFileSystem = (IFileSystem *)fnCreateInterface(FILESYSTEM_INTERFACE_VERSION, NULL);
}

void GetEngineModule(void)
{
	g_hEngineModule = GetModuleHandle("hw.dll");

	if (!g_hEngineModule)
		g_hEngineModule = GetModuleHandle("sw.dll");
}

void LoadMetaHook(void)
{
	int len = strlen(g_szModulePath) - 7;
	char szModuleName[MAX_PATH];
	strncpy(szModuleName, g_szModulePath, len);
	szModuleName[len] = 0;
	strcat(szModuleName, "metahook.dll");

	MH_LoadLibrary(szModuleName);
}

BOOL WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpvReserved)
{
	g_hThisModule = hModule;

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		g_hClientModule = NULL;
		g_bIsUseSteam = false;
		g_bMetaHook = false;

		memset(&g_blobfootprintClient, 0, sizeof(g_blobfootprintClient));
		memset(&g_modfuncs, 0, sizeof(g_modfuncs));
		memset(&gExportfuncs, 0, sizeof(gExportfuncs));

		for (int i = 0; i < sizeof(g_cldstAddrs); i += 4)
			*(void (**)(void))((DWORD)&g_cldstAddrs + i) = EmptyFunction;

		gExportfuncs.Initialize = (INITIALIZE_FUNC)&g_modfuncs;
		gExportfuncs.HUD_VidInit = (HUD_VIDINIT_FUNC)&g_modfuncs;

		GetModulePath();
		GetFileSystem();

		registry->Init();
		CommandLine()->CreateCmdLine(GetCommandLine());
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		if (g_hClientModule)
			FreeLibrary(g_hClientModule);

		registry->Shutdown();
	}

	return TRUE;
}

#define EXPORT __declspec(dllexport)

extern "C"
{
	EXPORT int Initialize(struct cl_enginefuncs_s *pEnginefuncs, int iVersion);
	EXPORT void HUD_Init(void);
	EXPORT int HUD_VidInit(void);
	EXPORT int HUD_Redraw(float time, int intermission);
	EXPORT int HUD_UpdateClientData(struct client_data_s *pcldata, float flTime);
	EXPORT void HUD_Reset(void);
	EXPORT void HUD_PlayerMove(struct playermove_s *ppmove, int server);
	EXPORT void HUD_PlayerMoveInit(struct playermove_s *ppmove);
	EXPORT char HUD_PlayerMoveTexture(char *name);
	EXPORT void IN_ActivateMouse(void);
	EXPORT void IN_DeactivateMouse(void);
	EXPORT void IN_MouseEvent(int mstate);
	EXPORT void IN_ClearStates(void);
	EXPORT void IN_Accumulate(void);
	EXPORT void CL_CreateMove(float frametime, struct usercmd_s *cmd, int active);
	EXPORT int CL_IsThirdPerson(void);
	EXPORT void CL_CameraOffset(float *ofs);
	EXPORT struct kbutton_s *KB_Find(const char *name);
	EXPORT void CAM_Think(void);
	EXPORT void V_CalcRefdef(struct ref_params_s *pparams);
	EXPORT int HUD_AddEntity(int type, struct cl_entity_s *ent, const char *modelname);
	EXPORT void HUD_CreateEntities(void);
	EXPORT void HUD_DrawNormalTriangles(void);
	EXPORT void HUD_DrawTransparentTriangles(void);
	EXPORT void HUD_StudioEvent(const struct mstudioevent_s *event, const struct cl_entity_s *entity);
	EXPORT void HUD_PostRunCmd(struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed);
	EXPORT void HUD_Shutdown(void);
	EXPORT void HUD_TxferLocalOverrides(struct entity_state_s *state, const struct clientdata_s *client);
	EXPORT void HUD_ProcessPlayerState(struct entity_state_s *dst, const struct entity_state_s *src);
	EXPORT void HUD_TxferPredictionData(struct entity_state_s *ps, const struct entity_state_s *pps, struct clientdata_s *pcd, const struct clientdata_s *ppcd, struct weapon_data_s *wd, const struct weapon_data_s *pwd);
	EXPORT void Demo_ReadBuffer(int size, unsigned char *buffer);
	EXPORT int HUD_ConnectionlessPacket(const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size);
	EXPORT int HUD_GetHullBounds(int hullnumber, float *mins, float *maxs);
	EXPORT void HUD_Frame(double time);
	EXPORT int HUD_Key_Event(int eventcode, int keynum, const char *pszCurrentBinding);
	EXPORT void HUD_TempEntUpdate(double frametime, double client_time, double cl_gravity, struct tempent_s **ppTempEntFree, struct tempent_s **ppTempEntActive, int (*Callback_AddVisibleEntity)(struct cl_entity_s *pEntity), void (*Callback_TempEntPlaySound)(struct tempent_s *pTemp, float damp));
	EXPORT struct cl_entity_s *HUD_GetUserEntity(int index);
	EXPORT void HUD_VoiceStatus(int entindex, qboolean bTalking);
	EXPORT void HUD_DirectorMessage(int iSize, void *pbuf);
	EXPORT int HUD_GetStudioModelInterface(int version, struct r_studio_interface_s **ppinterface, struct engine_studio_api_s *pstudio);
	EXPORT void HUD_ChatInputPosition(int *x, int *y);
	EXPORT int HUD_GetPlayerTeam(int player);
	EXPORT void *ClientFactory(void);
};

extern "C" EXPORT void F(void *pv)
{
	const char *cmdline = GetCommandLine();

	if (strstr(cmdline, "-steam") != NULL)
	{
		const char *result = strstr(cmdline, "-steam");

		if (result[6] == ' ' || result[6] == '\0')
			g_bIsUseSteam = true;
	}
	else
		g_bIsUseSteam = false;

	if (g_bIsUseSteam)
	{
		char szModuleName[MAX_PATH];
		GetModuleFileName(GetModuleHandle(NULL), szModuleName, sizeof(szModuleName));

		char *pszExeName = strrchr(szModuleName, '\\') + 1;

		if (!stricmp(pszExeName, "hl.exe"))
			g_bMetaHook = true;
	}

	if (g_bMetaHook)
	{
		LoadMetaHook();

		if (!g_hEngineModule)
			GetEngineModule();

		if (g_hEngineModule)
		{
			const char *pszGameName;
			CommandLine()->CheckParm("-game", &pszGameName);

			MH_Setup(g_pFileSystem, CommandLine(), registry);
			MH_Init(pszGameName);
			MH_LoadEngine(g_hEngineModule);
		}
	}

	if (!g_hClientModule)
	{
		char szModuleName[MAX_PATH];
		sprintf(szModuleName, "%s\\new_client.dll", g_szModulePath);

		g_hClientModule = LoadLibrary(szModuleName);
	}

	if (g_hClientModule)
	{
		void (*pfnF)(void *pv) = (void (*)(void *))GetProcAddress(g_hClientModule, "F");

		if (pfnF)
		{
			pfnF(pv);
			gpExportfuncs = (cl_exportfuncs_t *)pv;

			if (gpExportfuncs)
			{
				memcpy(&gExportfuncs, gpExportfuncs, sizeof(gExportfuncs));

				if (g_bMetaHook)
					MH_LoadClient(gpExportfuncs, &gExportfuncs);
			}
		}
	}
}

int Initialize(struct cl_enginefuncs_s *pEnginefuncs, int iVersion)
{
	memcpy(&gEngfuncs, pEnginefuncs, sizeof(gEngfuncs));

	if (!g_bIsUseSteam)
	{
		char szModuleName[MAX_PATH];
		GetModuleFileName(GetModuleHandle(NULL), szModuleName, sizeof(szModuleName));

		if (!stricmp(&szModuleName[strlen(szModuleName) - 1 - sizeof("hl.exe") - 1], "hl.exe"))
			g_bIsUseSteam = (gEngfuncs.CheckParm("-steam", NULL) != 0);

		if (!g_bIsUseSteam)
		{
			sprintf(szModuleName, "%s\\blob_client.dll", g_szModulePath);
			NLoadBlobFile(szModuleName, &g_blobfootprintClient, (void **)&gExportfuncs);

			if (g_bMetaHook)
				MH_LoadClient(gpExportfuncs, &gExportfuncs);
		}
		else
		{
		}
	}

	return gExportfuncs.Initialize(pEnginefuncs, iVersion);
}

void HUD_Init(void)
{
	gExportfuncs.HUD_Init();
}

int HUD_VidInit(void)
{
	return gExportfuncs.HUD_VidInit();
}

int HUD_Redraw(float time, int intermission)
{
	return gExportfuncs.HUD_Redraw(time, intermission);
}

int HUD_UpdateClientData(struct client_data_s *pcldata, float flTime)
{
	return gExportfuncs.HUD_UpdateClientData(pcldata, flTime);
}

void HUD_Reset(void)
{
	gExportfuncs.HUD_Reset();
}

void HUD_PlayerMove(struct playermove_s *ppmove, int server)
{
	gExportfuncs.HUD_PlayerMove(ppmove, server);
}

void HUD_PlayerMoveInit(struct playermove_s *ppmove)
{
	gExportfuncs.HUD_PlayerMoveInit(ppmove);
}

char HUD_PlayerMoveTexture(char *name)
{
	return gExportfuncs.HUD_PlayerMoveTexture(name);
}

void IN_ActivateMouse(void)
{
	gExportfuncs.IN_ActivateMouse();
}

void IN_DeactivateMouse(void)
{
	gExportfuncs.IN_DeactivateMouse();
}

void IN_MouseEvent(int mstate)
{
	gExportfuncs.IN_MouseEvent(mstate);
}

void IN_ClearStates(void)
{
	gExportfuncs.IN_ClearStates();
}

void IN_Accumulate(void)
{
	gExportfuncs.IN_Accumulate();
}

void CL_CreateMove(float frametime, struct usercmd_s *cmd, int active)
{
	gExportfuncs.CL_CreateMove(frametime, cmd, active);
}

int CL_IsThirdPerson(void)
{
	return gExportfuncs.CL_IsThirdPerson();
}

void CL_CameraOffset(float *ofs)
{
	gExportfuncs.CL_CameraOffset(ofs);
}

struct kbutton_s *KB_Find(const char *name)
{
	return gExportfuncs.KB_Find(name);
}

void CAM_Think(void)
{
	gExportfuncs.CAM_Think();
}

void V_CalcRefdef(struct ref_params_s *pparams)
{
	gExportfuncs.V_CalcRefdef(pparams);
}

int HUD_AddEntity(int type, struct cl_entity_s *ent, const char *modelname)
{
	return gExportfuncs.HUD_AddEntity(type, ent, modelname);
}

void HUD_CreateEntities(void)
{
	gExportfuncs.HUD_CreateEntities();
}

void HUD_DrawNormalTriangles(void)
{
	gExportfuncs.HUD_DrawNormalTriangles();
}

void HUD_DrawTransparentTriangles(void)
{
	gExportfuncs.HUD_DrawTransparentTriangles();
}

void HUD_StudioEvent(const struct mstudioevent_s *event, const struct cl_entity_s *entity)
{
	gExportfuncs.HUD_StudioEvent(event, entity);
}

void HUD_PostRunCmd(struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed)
{
	gExportfuncs.HUD_PostRunCmd(from, to, cmd, runfuncs, time, random_seed);
}

void HUD_Shutdown(void)
{
	gExportfuncs.HUD_Shutdown();
}

void HUD_TxferLocalOverrides(struct entity_state_s *state, const struct clientdata_s *client)
{
	gExportfuncs.HUD_TxferLocalOverrides(state, client);
}

void HUD_ProcessPlayerState(struct entity_state_s *dst, const struct entity_state_s *src)
{
	gExportfuncs.HUD_ProcessPlayerState(dst, src);
}

void HUD_TxferPredictionData(struct entity_state_s *ps, const struct entity_state_s *pps, struct clientdata_s *pcd, const struct clientdata_s *ppcd, struct weapon_data_s *wd, const struct weapon_data_s *pwd)
{
	gExportfuncs.HUD_TxferPredictionData(ps, pps, pcd, ppcd, wd, pwd);
}

void Demo_ReadBuffer(int size, unsigned char *buffer)
{
	gExportfuncs.Demo_ReadBuffer(size, buffer);
}

int HUD_ConnectionlessPacket(const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size)
{
	return gExportfuncs.HUD_ConnectionlessPacket(net_from, args, response_buffer, response_buffer_size);
}

int HUD_GetHullBounds(int hullnumber, float *mins, float *maxs)
{
	return gExportfuncs.HUD_GetHullBounds(hullnumber, mins, maxs);
}

void HUD_Frame(double time)
{
	gExportfuncs.HUD_Frame(time);
}

int HUD_Key_Event(int eventcode, int keynum, const char *pszCurrentBinding)
{
	return gExportfuncs.HUD_Key_Event(eventcode, keynum, pszCurrentBinding);
}

void HUD_TempEntUpdate(double frametime, double client_time, double cl_gravity, struct tempent_s **ppTempEntFree, struct tempent_s **ppTempEntActive, int (*Callback_AddVisibleEntity)(struct cl_entity_s *pEntity), void (*Callback_TempEntPlaySound)(struct tempent_s *pTemp, float damp))
{
	gExportfuncs.HUD_TempEntUpdate(frametime, client_time, cl_gravity, ppTempEntFree, ppTempEntActive, Callback_AddVisibleEntity, Callback_TempEntPlaySound);
}

struct cl_entity_s *HUD_GetUserEntity(int index)
{
	return gExportfuncs.HUD_GetUserEntity(index);
}

void HUD_VoiceStatus(int entindex, qboolean bTalking)
{
	gExportfuncs.HUD_VoiceStatus(entindex, bTalking);
}

void HUD_DirectorMessage(int iSize, void *pbuf)
{
	gExportfuncs.HUD_DirectorMessage(iSize, pbuf);
}

int HUD_GetStudioModelInterface(int version, struct r_studio_interface_s **ppinterface, struct engine_studio_api_s *pstudio)
{
	return gExportfuncs.HUD_GetStudioModelInterface(version, ppinterface, pstudio);
}

void HUD_ChatInputPosition(int *x, int *y)
{
	gExportfuncs.HUD_ChatInputPosition(x, y);
}

int HUD_GetPlayerTeam(int player)
{
	return gExportfuncs.HUD_GetPlayerTeam(player);
}

void *ClientFactory(void)
{
	return gExportfuncs.ClientFactory();
}

EXPORT_FUNCTION IBaseInterface *CreateInterface(const char *pName, int *pReturnCode)
{
	if (gExportfuncs.ClientFactory)
		return ((CreateInterfaceFn)gExportfuncs.ClientFactory())(pName, pReturnCode);

	if (g_hClientModule)
		return ((CreateInterfaceFn)GetProcAddress(g_hClientModule, CREATEINTERFACE_PROCNAME))(pName, pReturnCode);

	return NULL;
}