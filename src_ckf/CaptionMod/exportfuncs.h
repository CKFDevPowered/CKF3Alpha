#pragma once
#include <gl/gl.h>
#include <const.h>
#include <triangleapi.h>
#include <cl_entity.h>
#include <event_api.h>
#include <ref_params.h>
#include <com_model.h>
#include <cvardef.h>
#include <r_efx.h>
#include <r_studioint.h>
#include <pm_movevars.h>
#include <studio.h>
#include <entity_types.h>
#include <usercmd.h>

extern cl_enginefunc_t gEngfuncs;
extern engine_studio_api_t IEngineStudio;
extern engine_studio_api_t *gpEngineStudio;
extern r_studio_interface_t StudioInterface;
extern ref_params_t refparams;
extern cl_entity_t **CurrentEntity;
extern studiohdr_t **StudioHeader;
extern int *cls_viewmodel_sequence;
extern int *cls_viewmodel_body;
extern float *cls_viewmodel_starttime;

extern int (*g_pfnAddVisibleEntity)(cl_entity_t *);
extern int g_fLOD;
extern int g_iLODLevel;
extern int g_bRenderPlayerWeapon;
extern double g_flFrameTime;
extern double g_flClientTime;
extern int g_iHookSetupBones;
extern int g_iViewModelSkin;
extern int g_iViewModelBody;
extern float g_flTraceDistance;
extern cl_entity_t *g_pTraceEntity;
extern qboolean g_iHudVidInitalized;
extern int g_RefSupportExt;
extern cl_entity_t *cl_viewent;

extern HWND g_hWnd;

void QGL_Init(void);
void Cvar_Init(void);
void HudBase_Init(void);
int HudBase_VidInit(void);
int HudBase_Redraw(float flTime, int iIntermission);
int HudBase_KeyEvent(int eventcode, int keynum, const char *pszCurrentBinding);
void HudWeaponAnim(int iSequence);
void HudWeaponAnimEx(int iSequence, int iBody, int iSkin, float flAnimtime);
void EV_HookEvents(void);

int Initialize(struct cl_enginefuncs_s *pEnginefuncs, int iVersion);
void HUD_Init(void);
int HUD_VidInit(void);
int HUD_Redraw(float time, int intermission);
void IN_Accumulate(void);
int HUD_Key_Event( int eventcode, int keynum, const char *pszCurrentBinding );
void IN_MouseEvent(int mstate);
int HUD_GetStudioModelInterface( int version, struct r_studio_interface_s **ppinterface, struct engine_studio_api_s *pstudio );
void V_CalcRefdef(struct ref_params_s *pparams);
void HUD_TempEntUpdate(double frametime, double client_time, double cl_gravity, struct tempent_s **ppTempEntFree, struct tempent_s **ppTempEntActive, 	int (*pfnAddVisibleEntity)(cl_entity_t *),	void (*pfnTempEntPlaySound)( TEMPENTITY *, float damp));
void HUD_DrawNormalTriangles(void);
int HUD_AddEntity(int iType, struct cl_entity_s *pEntity, const char *szModel);
int HUD_UpdateClientData(client_data_t *cldata, float flTime);
void HUD_DrawTransparentTriangles(void);
void HUD_PostRunCmd(local_state_t *from, local_state_t *to, usercmd_t *cmd, int runfuncs, double time, unsigned int random_seed);
void HUD_ProcessPlayerState( struct entity_state_s *dst, const struct entity_state_s *src );
void CL_CreateMove ( float frametime, struct usercmd_s *cmd, int active );
void HUD_WeaponsPostThink( local_state_s *from, local_state_s *to, usercmd_t *cmd, double time, unsigned int random_seed );
void HUD_StudioEvent( const struct mstudioevent_s *ev, const struct cl_entity_s *pEntity );

extern "C"
{
	void PM_Init(struct playermove_s *ppmove);
	void PM_Move(struct playermove_s *ppmove, int server);
	char PM_FindTextureType(char *name);
}
void HUD_PlayerMove(struct playermove_s *ppmove, int server);
void HUD_PlayerMoveInit(struct playermove_s *ppmove);
char HUD_PlayerMoveTexture(char *name);
void Hook_SV_StudioSetupBones(model_t *pModel, float frame, int sequence, vec_t *angles, vec_t *origin, const byte *pcontroller, const byte *pblending, int iBone, const edict_t *edict);