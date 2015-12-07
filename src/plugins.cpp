#include <metahook.h>
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "model.h"
#include "steam_api.h"
#include "configs.h"
#include "Renderer.h"
#include "FileSystem.h"
#include "BaseUI.h"
#include "SecureClient.h"
#include "Video.h"
#include "perf_counter.h"
#include "modules.h"
#include "hooks.h"
#include "mempatchs.h"
#include "zone.h"
#include "demo.h"
#include "kbutton.h"
#include "camera.h"
#include "inputw32.h"
#include "view.h"
#include "Surface.h"
#include "cvar.h"
#include "vgui_int.h" 
#include "qgl.h"

cl_exportfuncs_t gExportfuncs;
mh_interface_t *g_pInterface;
metahook_api_t *g_pMetaHookAPI;
mh_enginesave_t *g_pMetaSave;

HINSTANCE g_hInstance, g_hThisModule, g_hEngineModule;
DWORD g_dwEngineBase, g_dwEngineSize;
DWORD g_dwEngineBuildnum;
DWORD g_iVideoMode;
int g_iVideoWidth, g_iVideoHeight, g_iBPP;
bool g_bWindowed;
bool g_bIsUseSteam;
bool g_bIsRunningSteam;
bool g_bIsDebuggerPresent;
IFileSystem *g_pFileSystem;

void (*g_pfnHUD_VoiceStatus)(int entindex, qboolean bTalking);

void IPlugins::Init(metahook_api_t *pAPI, mh_interface_t *pInterface, mh_enginesave_t *pSave)
{
	BOOL (*IsDebuggerPresent)(void) = (BOOL (*)(void))GetProcAddress(GetModuleHandle("kernel32.dll"), "IsDebuggerPresent");

	g_pInterface = pInterface;
	g_pMetaHookAPI = pAPI;
	g_pMetaSave = pSave;
	g_hInstance = GetModuleHandle(NULL);
	g_bIsUseSteam = CommandLine()->CheckParm("-steam") != NULL;
	g_bIsRunningSteam = SteamAPI_IsSteamRunning();
	g_bIsDebuggerPresent = IsDebuggerPresent() != FALSE;

	Config_Init();
	Audio_Init();
	Renderer_Init();
	CKF_Init();

	//CommandLine()->AppendParm("-nocdaudio", NULL);
	CommandLine()->AppendParm("-nomaster", NULL);
	CommandLine()->AppendParm("-insecure", NULL);
	CommandLine()->AppendParm("-forcevalve", NULL);

	gPerformanceCounter.InitializePerformanceCounter();

	if (g_pAudioPlugins)
		g_pAudioPlugins->Init(pAPI, pInterface, pSave);

	if (g_pRendererPlugins)
		g_pRendererPlugins->Init(pAPI, pInterface, pSave);

	if (g_pCKFClientPlugins)
		g_pCKFClientPlugins->Init(pAPI, pInterface, pSave);
}

void IPlugins::Shutdown(void)
{
	if (g_pAudioPlugins)
		g_pAudioPlugins->Shutdown();
	if (g_pRendererPlugins)
		g_pRendererPlugins->Shutdown();
	if (g_pCKFClientPlugins)
		g_pCKFClientPlugins->Shutdown();

	CKF_Shutdown();
	Renderer_Shutdown();
	Audio_Shutdown();	
	Module_Shutdown();
}

void IPlugins::LoadEngine(void)
{
	g_pFileSystem = g_pInterface->FileSystem;
	g_iVideoMode = g_pMetaHookAPI->GetVideoMode(&g_iVideoWidth, &g_iVideoHeight, &g_iBPP, &g_bWindowed);
	g_dwEngineBuildnum = g_pMetaHookAPI->GetEngineBuildnum();
	g_hEngineModule = g_pMetaHookAPI->GetEngineModule();
	g_dwEngineBase = g_pMetaHookAPI->GetEngineBase();
	g_dwEngineSize = g_pMetaHookAPI->GetEngineSize();

	if (g_pAudioPlugins)
		g_pAudioPlugins->LoadEngine();

	if (g_pRendererPlugins)
		g_pRendererPlugins->LoadEngine();

	if (g_pCKFClientPlugins)
		g_pCKFClientPlugins->LoadEngine();

	//QGL_Init is brought forward to called in order to draw loading screen
	if(g_dwEngineBuildnum >= 5953)
		QGL_Init();

	InstallHook();
	GetEngfuncs();

	MemPatch_Start(MEMPATCH_STEP_LOADENGINE);
	Memory_Init();
	Cvar_Init();
	SteamAPI_Load();
	SteamAPI_Init();

	FileSystem_InstallHook(g_pInterface->FileSystem);
	VID_InstallHook();
	BaseUI_InstallHook();
	Surface_InstallHook();
	Module_InstallHook();
	SecureClient_InstallHook();
}

void IPlugins::LoadClient(cl_exportfuncs_t *pExportFunc)
{
	memcpy(&gExportfuncs, pExportFunc, sizeof(gExportfuncs));

	pExportFunc->Initialize = Initialize;
	pExportFunc->ClientFactory = ClientFactory;

	pExportFunc->HUD_Init = HUD_Init;
	pExportFunc->HUD_VidInit = HUD_VidInit;
	pExportFunc->HUD_Redraw = HUD_Redraw;
	pExportFunc->HUD_GetStudioModelInterface = HUD_GetStudioModelInterface;
	pExportFunc->HUD_ProcessPlayerState = HUD_ProcessPlayerState;
	pExportFunc->HUD_TxferLocalOverrides = HUD_TxferLocalOverrides;
	pExportFunc->HUD_PostRunCmd = HUD_PostRunCmd;
	pExportFunc->HUD_TxferPredictionData = HUD_TxferPredictionData;
	pExportFunc->HUD_DrawNormalTriangles = HUD_DrawNormalTriangles;
	pExportFunc->HUD_DrawTransparentTriangles = HUD_DrawTransparentTriangles;
	pExportFunc->HUD_UpdateClientData = HUD_UpdateClientData;
	pExportFunc->HUD_StudioEvent = HUD_StudioEvent;
	pExportFunc->HUD_Frame = HUD_Frame;
	pExportFunc->HUD_PlayerMove = HUD_PlayerMove;
	pExportFunc->HUD_PlayerMoveTexture = HUD_PlayerMoveTexture;
	pExportFunc->HUD_Shutdown = HUD_Shutdown;
	pExportFunc->HUD_TempEntUpdate = HUD_TempEntUpdate;
	pExportFunc->HUD_CreateEntities = HUD_CreateEntities;
	pExportFunc->HUD_PlayerMoveInit = HUD_PlayerMoveInit;
	pExportFunc->HUD_AddEntity = HUD_AddEntity;
	pExportFunc->HUD_ConnectionlessPacket = HUD_ConnectionlessPacket;
	pExportFunc->HUD_Key_Event = HUD_Key_Event;
	pExportFunc->HUD_DirectorMessage = HUD_DirectorMessage;

	pExportFunc->CL_CreateMove = CL_CreateMove;
	pExportFunc->CL_IsThirdPerson = CL_IsThirdPerson;

	pExportFunc->IN_ActivateMouse = IN_ActivateMouse;
	pExportFunc->IN_DeactivateMouse = IN_DeactivateMouse;
	pExportFunc->IN_MouseEvent = IN_MouseEvent;
	pExportFunc->IN_Accumulate = IN_Accumulate;
	pExportFunc->IN_ClearStates = IN_ClearStates;

	pExportFunc->CAM_Think = CAM_Think;
	pExportFunc->V_CalcRefdef = V_CalcRefdef;
	pExportFunc->Demo_ReadBuffer = Demo_ReadBuffer;
	pExportFunc->KB_Find = KB_Find;

	if (gExportfuncs.HUD_VoiceStatus)
		g_pMetaHookAPI->InlineHook(gExportfuncs.HUD_VoiceStatus, HUD_VoiceStatus, (void *&)g_pfnHUD_VoiceStatus);

	MemPatch_Start(MEMPATCH_STEP_LOADCLIENT);

	if (g_pAudioPlugins)
		g_pAudioPlugins->LoadClient(pExportFunc);

	if (g_pRendererPlugins)
		g_pRendererPlugins->LoadClient(pExportFunc);

	if (g_pCKFClientPlugins)
		g_pCKFClientPlugins->LoadClient(pExportFunc);
}

void IPlugins::ExitGame(int iResult)
{
	VID_Shutdown();
	VGui_Shutdown();

	if (g_pAudioPlugins)
		g_pAudioPlugins->ExitGame(iResult);

	if (g_pRendererPlugins)
		g_pRendererPlugins->ExitGame(iResult);

	if (g_pCKFClientPlugins)
		g_pCKFClientPlugins->ExitGame(iResult);
}

EXPOSE_SINGLE_INTERFACE(IPlugins, IPlugins, METAHOOK_PLUGIN_API_VERSION);