#pragma once

#include "plugins.h"
#include <usercmd.h>
#include <com_model.h>
#include <studio.h>
#include <pm_defs.h>
#include <tier0/basetypes.h>
#include <cvardef.h>
#include <ICKFClient.h>
#include "gl_model.h"
#include "ref_int.h"

#define GetEngfuncsAddress(addr) (g_dwEngineBase+addr-0x1D01000)
#define GetCallAddress(addr) (addr + (*(DWORD *)(addr+1)) + 5)

#define SIG_NOT_FOUND(name) Sys_ErrorEx(va("Unable to locate: %s\nEngine buildnum: %d", name, g_dwEngineBuildnum));
#define LIB_NOT_FOUND(name) Sys_ErrorEx(va("Couln't load: %s", name));

typedef struct
{	
	//engine
	void (*SV_StudioSetupBones)(model_t *pModel, float frame, int sequence, vec_t *angles, vec_t *origin, const byte *pcontroller, const byte *pblending, int iBone, const edict_t *edict);
	void (*S_StartDynamicSound)(int entnum, int entchannel, sfx_t *sfx, vec3_t origin, float fvol, float attenuation, int flags, int pitch);
	void (*S_StopSound)(int entnum, int entchannel);
	sfx_t *(*CL_LookupSound)(const char *name);
	void (*R_DrawViewModel)(void);
	//client
	void (__fastcall *ShowSpectatorGUI)(void *, int);
	void (__fastcall *ShowSpectatorGUIBar)(void *, int);
	const char *(__fastcall *GetMapName)(void *, int);
	const char *(__fastcall *GetServerName)(void *, int);
	void (__fastcall *StudioSaveBones)(void *, int);
	void (__fastcall *StudioCalcAttachments)(void *, int);
	void (*IN_MouseMove)(float frametime, usercmd_t *cmd);
	void (*V_CalcNormalRefdef)(struct ref_params_s *pparams);
	//api
	int (WINAPI *SetCursorPos)(int X, int Y);
	HWND (WINAPI *CreateWindowExA)(DWORD dwExStyle, LPCTSTR lpClassName, LPCTSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
	HWND (WINAPI *CreateWindowExW)(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
	BOOL (WINAPI *SetWindowText)(HWND hwnd, LPCTSTR lpString);
}hook_funcs_t;

extern hook_funcs_t gHookFuncs;
extern bte_funcs_t gBTEFuncs;

//win api
int WINAPI Hook_SetCursorPos(int X, int Y);//called from GameUI.dll->vgui2.dll
BOOL WINAPI Hook_SetWindowTextA(HWND hwnd, LPCTSTR lpString);

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
extern HWND g_hWnd;

void Sys_ErrorEx(const char *error);

void Hook_IN_MouseMove(float frametime, usercmd_t *cmd);
void Hook_R_DrawViewModel(void);
void Hook_SV_StudioSetupBones(model_t *pModel, float frame, int sequence, vec_t *angles, vec_t *origin, const byte *pcontroller, const byte *pblending, int iBone, const edict_t *edict);
HWND WINAPI Hook_CreateWindowExA(DWORD dwExStyle, LPCTSTR lpClassName, LPCTSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);
HWND WINAPI Hook_CreateWindowExW(DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int x, int y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam);