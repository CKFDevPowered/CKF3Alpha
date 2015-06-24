#include <metahook.h>
#include "exportfuncs.h"
#include "engfuncs.h"
#include "client.h"
#include "qgl.h"
#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"
#include "util.h"
#include "tent.h"
#include "cvar.h"
#include <pm_defs.h>
#include <pm_shared.h>

const char g_szGameName[] = "Chicken Fortress 3";

extern int g_mouse_state;
extern int g_mouse_oldstate;
extern bool g_bGameUIActivate;
extern cl_entity_t g_entTeamMenu[4];
extern cl_entity_t g_entClassMenu[2];

void HudBase_MouseUp(int mx, int my);
void HudBase_MouseDown(int mx, int my);
bool HudBase_IsFullScreenMenu(void);

void StudioEntityLight(struct alight_s *plight);
void StudioSetupModel(int bodypart, void **ppbodypart, void **ppsubmodel);
void StudioModelRenderer_InstallHook(void);
void CL_TraceEntity(void);
void SpyWatch_Draw(void);

void T_VidInit(void);
void T_UpdateTEnts(void);
void T_DrawTEnts(void);

void Renderer_Init(void);
void Cvar_HudInit(void);

void R_UpdateParticles(void);
void R_DrawParticles(void);
void R_Particles_VidInit(void);

void UpdateBuildables(void);

void UserMsg_InstallHook(void);

cl_exportfuncs_t gClientfuncs =
{
	Initialize,
	HUD_Init,
	HUD_VidInit,
	HUD_Redraw,
	HUD_UpdateClientData,
	NULL,
	HUD_PlayerMove,
	HUD_PlayerMoveInit,
	HUD_PlayerMoveTexture,
	NULL,
	NULL,
	IN_MouseEvent,
	NULL,
	NULL,
	CL_CreateMove,
	NULL,
	NULL,
	NULL,
	NULL,
	V_CalcRefdef,
	HUD_AddEntity,
	NULL,
	HUD_DrawNormalTriangles,
	HUD_DrawTransparentTriangles,
	HUD_StudioEvent,
	HUD_PostRunCmd,
	NULL,
	NULL,
	HUD_ProcessPlayerState,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	HUD_Key_Event,
	HUD_TempEntUpdate,
	NULL,
	NULL,
	NULL,
	HUD_GetStudioModelInterface,
	NULL,
	NULL,
	NULL
};

cl_enginefunc_t gEngfuncs;
engine_studio_api_t *gpEngineStudio;
engine_studio_api_t IEngineStudio;
r_studio_interface_t StudioInterface;
ref_params_t refparams;
cl_entity_t **CurrentEntity;
studiohdr_t **StudioHeader;

float *ev_punchangle;

void *gpViewPortInterface;

int *cls_viewmodel_sequence;
int *cls_viewmodel_body;
float *cls_viewmodel_starttime;

int g_bRenderPlayerWeapon;
int g_fLOD;
int g_iLODLevel;

int g_iViewModelBody;
int g_iViewModelSkin;

double g_flClientTime;
double g_flFrameTime;
int g_iHookSetupBones;
float g_flTraceDistance;
cl_entity_t *g_pTraceEntity;
qboolean g_iHudVidInitalized;
int g_RefSupportExt;
cl_entity_t *cl_viewent;

int Initialize(struct cl_enginefuncs_s *pEnginefuncs, int iVersion)
{
	memcpy(&gEngfuncs, pEnginefuncs, sizeof(gEngfuncs));

#define CL_VIEWMODEL_SEQUENCE_SIG "\xA3\x2A\x2A\x2A\x2A\xC7\x05"
	DWORD addr = (DWORD)g_pMetaHookAPI->SearchPattern((void *)g_pMetaSave->pEngineFuncs->pfnWeaponAnim, 0x50, CL_VIEWMODEL_SEQUENCE_SIG, sizeof(CL_VIEWMODEL_SEQUENCE_SIG)-1);
	if(!addr)
		SIG_NOT_FOUND("cl.viewmodel_sequence");

	cls_viewmodel_sequence = *(int **)(addr + 1);
	cls_viewmodel_starttime = *(float **)(addr + 7);
	cls_viewmodel_body = *(int **)(addr + 17);

	QGL_Init();
	Renderer_Init();

	return 1;
}

void HUD_Init(void)
{
	HudBase_Init();
	Cvar_Init();
	Cvar_HudInit();
	EV_HookEvents();
	UserMsg_InstallHook();
	
	//this is register after HUD_Init
	gHUD_m_pip = gEngfuncs.pfnGetCvarPointer("spec_pip");
}

int HUD_VidInit(void)
{
	T_VidInit();
	HudBase_VidInit();
	R_Particles_VidInit();

	g_iHudVidInitalized = true;

	cl_viewent = gEngfuncs.GetViewModel();

	gEngfuncs.pfnClientCmd("bind \",\" \"chooseclass\"");
	gEngfuncs.pfnClientCmd("bind \".\" \"chooseteam\"");

	return 1;
}

int HUD_Redraw(float time, int intermission)
{
	HudBase_Redraw(time, intermission);

	return 1;
}

void IN_Accumulate(void)
{
	if(HudBase_IsFullScreenMenu())
		return;

	//gExportfuncs.IN_Accumulate();
}

int HUD_Key_Event( int eventcode, int keynum, const char *pszCurrentBinding )
{
	if(HudBase_KeyEvent(eventcode, keynum, pszCurrentBinding))
		return 0;

	return 1;
}

void IN_MouseEvent(int mstate)
{
	if(HudBase_IsFullScreenMenu())
	{
		g_mouse_oldstate = g_mouse_state;
		g_mouse_state = mstate;

		int mx, my;
		
		gEngfuncs.GetMousePosition(&mx, &my);
		if(g_mouse_state == 0 && g_mouse_oldstate == 1)
		{
			HudBase_MouseUp(mx, my);
		}
		else if(g_mouse_state == 1 && g_mouse_oldstate == 0)
		{
			HudBase_MouseDown(mx, my);
		}
	}
}

void Hook_IN_MouseMove(float frametime, usercmd_t *cmd)
{
	if(HudBase_IsFullScreenMenu())
		return;
	gHookFuncs.IN_MouseMove(frametime, cmd);
}

r_studio_interface_t studio_interface =
{
	STUDIO_INTERFACE_VERSION,
	R_StudioDrawModel,
	R_StudioDrawPlayer,
};

int HUD_GetStudioModelInterface( int version, struct r_studio_interface_s **ppinterface, struct engine_studio_api_s *pstudio )
{
	memcpy(&IEngineStudio, pstudio, sizeof(engine_studio_api_t));
	CurrentEntity = *(cl_entity_t ***)((DWORD)pstudio->GetCurrentEntity + 0x1);
	StudioHeader = *(studiohdr_t ***)((DWORD)pstudio->StudioSetHeader + 0x5);

	pstudio->StudioEntityLight = StudioEntityLight;
	pstudio->StudioSetupModel = StudioSetupModel;

	gpEngineStudio = pstudio;
	
	*ppinterface = &studio_interface;

	R_StudioInit();

	g_bRenderPlayerWeapon = 0;
	return 1;
}

void V_CalcRefdef(struct ref_params_s *pparams)
{
	if(HudBase_IsFullScreenMenu() && !g_bGameUIActivate)
	{
		pparams->viewangles[0] = 0;
		pparams->viewangles[1] = 0;
		pparams->viewangles[2] = 0;
		pparams->vieworg[0] = 0;
		pparams->vieworg[1] = 0;
		pparams->vieworg[2] = 0;
	}
	memcpy(&refparams, pparams, sizeof(ref_params_t));
	CL_TraceEntity();
}

void HUD_TempEntUpdate(double frametime, double client_time, double cl_gravity, struct tempent_s **ppTempEntFree, struct tempent_s **ppTempEntActive, 	int (*pfnAddVisibleEntity)(cl_entity_t *),	void (*pfnTempEntPlaySound)( TEMPENTITY *, float damp))
{
	g_flClientTime = client_time;
	g_flFrameTime = frametime;

	UpdateBuildables();
	R_UpdateParticles();
	T_UpdateTEnts();
}

void StudioEntityLight(struct alight_s *plight)
{
	if((*CurrentEntity)->player)
	{
		if(((*CurrentEntity)->curstate.effects & EF_INVULNERABLE) && !g_bRenderPlayerWeapon)
		{
			plight->ambientlight = 128;
			plight->shadelight = 192;
		}
	}
	if( (*CurrentEntity)->curstate.effects & EF_3DMENU )
	{
		plight->ambientlight = (*CurrentEntity)->curstate.iuser1;
		plight->shadelight = (*CurrentEntity)->curstate.iuser2;
		plight->color[0] = 1.0f;
		plight->color[1] = 1.0f;
		plight->color[2] = 1.0f;

		plight->plightvec[0] = 0;
		plight->plightvec[1] = 1;
		plight->plightvec[2] = -1;
	}
	IEngineStudio.StudioEntityLight(plight);
}

void HUD_DrawNormalTriangles(void)
{
	T_DrawTEnts();
}

void HUD_DrawTransparentTriangles(void)
{
	R_DrawParticles();
}

BOOL WINAPI Hook_SetWindowTextA(HWND hwnd, LPCTSTR lpString)
{
	if(hwnd == g_hWnd)
	{
		return gHookFuncs.SetWindowText(hwnd, g_szGameName);
	}
	return gHookFuncs.SetWindowText(hwnd, lpString);
}

int WINAPI Hook_SetCursorPos(int X, int Y)
{
	if(HudBase_IsFullScreenMenu())
		return 1;
	return gHookFuncs.SetCursorPos(X, Y);
}

HWND WINAPI Hook_CreateWindowExA(DWORD dwExStyle, LPCTSTR lpClassName, LPCTSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	if((DWORD)lpClassName > 0xFFFF && (!strcmp(lpClassName, "Valve001") || !strcmp(lpClassName, "SDL_app")) && !hWndParent)//not an atom string
	{
		g_hWnd = gHookFuncs.CreateWindowExA(dwExStyle, lpClassName, g_szGameName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
		return g_hWnd;
	}
	return gHookFuncs.CreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

HWND WINAPI Hook_CreateWindowExW(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	if((DWORD)lpClassName > 0xFFFF && (!wcscmp(lpClassName, L"Valve001") || !wcscmp(lpClassName, L"SDL_app")) && !hWndParent)//not an atom string
	{
		g_hWnd = gHookFuncs.CreateWindowExW(dwExStyle, lpClassName, ANSIToUnicode(g_szGameName), dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
		return g_hWnd;
	}
	return gHookFuncs.CreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, x, y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
}

void R_UpdateViewModel(void)
{
	*CurrentEntity = cl_viewent;
	cl_viewent->curstate.skin = CL_GetViewSkin();
	cl_viewent->curstate.body = CL_GetViewBody();
	cl_viewent->curstate.rendermode = kRenderNormal;
	cl_viewent->curstate.renderfx = kRenderFxNone;

	if(g_SpyWatch.show)
	{
		cl_viewent->curstate.renderfx = kRenderFxCloak;
		cl_viewent->curstate.renderamt = g_SpyWatch.ent.curstate.renderamt;
	}
}

void Hook_R_DrawViewModel(void)
{
	R_UpdateViewModel();

	if(CL_CanDrawViewModel())
	{
		gHookFuncs.R_DrawViewModel();

		SpyWatch_Draw();
	}
}

int HUD_AddEntity(int iType, struct cl_entity_s *pEntity, const char *szModel)
{
	if(strstr(szModel, "sniperdot"))
	{
		if(pEntity->curstate.owner == gEngfuncs.GetLocalPlayer()->index)
		{
			pEntity->curstate.renderamt = 0;
			pEntity->curstate.effects |= EF_NODRAW;
		}
	}

	return 1;
}

void StudioSetupModel(int bodypart, void **ppbodypart, void **ppsubmodel)
{
	if(!g_fLOD)
	{
		IEngineStudio.StudioSetupModel(bodypart, ppbodypart, ppsubmodel);
		return;
	}
	cl_entity_t *pEntity = (*CurrentEntity);
	int iSaveBody = (*CurrentEntity)->curstate.body;
	studiohdr_t *header = (*StudioHeader);
	bodypart = bodypart % header->numbodyparts;
	mstudiobodyparts_t *pbodypart = (mstudiobodyparts_t *)((char *)header + header->bodypartindex) + bodypart;
	if(pEntity->player)//player model
	{
		if(g_bRenderPlayerWeapon)//player weapon body
		{
			(*CurrentEntity)->curstate.body = (*CurrentEntity)->curstate.scale + g_iLODLevel;
		}
		else if(g_iLODLevel > 0 && bodypart == 1)
		{
			(*CurrentEntity)->curstate.body += g_iLODLevel * pbodypart->base;
		}
	}
	else if(g_iLODLevel > 0)
	{
		(*CurrentEntity)->curstate.body += g_iLODLevel * pbodypart->base;
	}
	IEngineStudio.StudioSetupModel(bodypart, ppbodypart, ppsubmodel);
	(*CurrentEntity)->curstate.body = iSaveBody;
}

int HUD_UpdateClientData(client_data_t *cldata, float flTime)
{
	if(g_iForceFOV > 0)
	{
		cldata->fov = g_iForceFOV;
	}
	return 1;
}

void HudWeaponAnim(int iSequence)
{
	*cls_viewmodel_sequence = iSequence;
	*cls_viewmodel_starttime = 0;
}

void HudWeaponAnimEx(int iSequence, int iBody, int iSkin, float flAnimtime)
{
	if(iSequence != -1)
	{
		*cls_viewmodel_sequence = iSequence;
		*cls_viewmodel_starttime = flAnimtime;
	}
	if(iBody != -1)
		g_iViewModelBody = iBody;
	if(iSkin != -1)
		g_iViewModelSkin = iSkin;
}

void HUD_PlayerMove(struct playermove_s *ppmove, int server)
{
	PM_Move(ppmove, server);
}

void HUD_PlayerMoveInit(struct playermove_s *ppmove)
{
	cl_pmove = ppmove;
	PM_Init(ppmove);
}

char HUD_PlayerMoveTexture(char *name)
{
	return PM_FindTextureType(name);
}

void Hook_SV_StudioSetupBones(model_t *pModel, float frame, int sequence, vec_t *angles, vec_t *origin, const byte *pcontroller, const byte *pblending, int iBone, const edict_t *edict)
{
	if(g_iHookSetupBones && strstr(pModel->name, "player"))
	{
		g_StudioRenderer.PM_StudioSetupBones(sequence);//hack hack
		return;
	}
	gHookFuncs.SV_StudioSetupBones(pModel, frame, sequence, angles, origin, pcontroller, pblending, iBone, edict);
}

void HUD_ProcessPlayerState( struct entity_state_s *dst, const struct entity_state_s *src )
{
	cl_entity_t *player = gEngfuncs.GetLocalPlayer();
	if ( dst->number == player->index )
	{
		g_iUser1 = src->iuser1;
		g_iUser2 = src->iuser2;
		g_iUser3 = src->iuser3;
	}
}

void CL_CreateMove ( float frametime, struct usercmd_s *cmd, int active )
{
	if(g_WeaponSelect)
	{
		cmd->weaponselect = g_WeaponSelect;
		g_WeaponSelect = 0;
	}

	if(g_ScoreBoardEnabled)
		cmd->buttons |= IN_SCORE;
}

void R_ShotgunMuzzle(cl_entity_t *pEntity, int attachment);

void HUD_StudioEvent( const struct mstudioevent_s *ev, const struct cl_entity_s *pEntity )
{
	switch( ev->event )
	{
	case 5001:
		//R_ShotgunMuzzle( (cl_entity_t *)pEntity, 0 );
		break;
	case 5011:
		//R_ShotgunMuzzle( (cl_entity_t *)pEntity, 1 );
		break;
	case 5021:
		//R_ShotgunMuzzle( (cl_entity_t *)pEntity, 2 );
		break;
	case 5031:
		//R_ShotgunMuzzle( (cl_entity_t *)pEntity, 3 );
		break;
	case 5002:
		gEngfuncs.pEfxAPI->R_SparkEffect( (float *)&pEntity->attachment[0], atoi( ev->options), -100, 100 );
		break;
	// Client side sound
	case 5004:		
		gEngfuncs.pfnPlaySoundByNameAtLocation( (char *)ev->options, 1.0, (float *)&pEntity->attachment[0] );
		break;
	default:
		break;
	}
}