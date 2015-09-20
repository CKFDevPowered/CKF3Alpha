#pragma once

#include "plugins.h"
#include <usercmd.h>
#include <com_model.h>
#include <studio.h>
#include <pm_defs.h>
#include <tier0/basetypes.h>
#include <cvardef.h>
#include <ICKFClient.h>
#include <IBTEClient.h>
#include "gl_model.h"
#include "ref_int.h"

#define GetEngfuncsAddress(addr) (g_dwEngineBase+addr-0x1D01000)
#define GetCallAddress(addr) (addr + (*(DWORD *)(addr+1)) + 5)

#define SIG_NOT_FOUND(name) Sys_ErrorEx("Could not found: %s\nEngine buildnum: %d", name, g_dwEngineBuildnum);
#define LIB_NOT_FOUND(name) Sys_ErrorEx("Could not load: %s", name);

typedef struct
{	
	void (*SV_StudioSetupBones)(model_t *pModel, float frame, int sequence, vec_t *angles, vec_t *origin, const byte *pcontroller, const byte *pblending, int iBone, const edict_t *edict);
	void (*R_DrawViewModel)(void);
	void (*Sys_Error)(const char *fmt, ...);
	model_t *(*Mod_LoadModel)(model_t *mod, qboolean crash, qboolean trackCRC);
}hook_funcs_t;

extern hook_funcs_t gHookFuncs;

//win api

//client.dll
extern int g_iUser1;
extern int g_iUser2;
extern int g_iUser3;
extern cvar_t *gHUD_m_pip;

//hw.dll
extern refdef_t *refdef;
extern playermove_t *cl_pmove;
extern int *envmap;
extern float *r_blend;

//for api

void Sys_ErrorEx(const char *error, ...);

void __fastcall Hook_LoadStartupGraphic(void *pthis, int);
void __fastcall Hook_DrawStartupGraphic(void *pthis, int, HWND hWnd);
void Hook_SV_StudioSetupBones(model_t *pModel, float frame, int sequence, vec_t *angles, vec_t *origin, const byte *pcontroller, const byte *pblending, int iBone, const edict_t *edict);