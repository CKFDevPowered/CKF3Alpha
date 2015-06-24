#include <metahook.h>

#include "cvar.h"
#include "sys.h"
#include "plugins.h"
#include "video.h"
#include "common.h"

#include "IBaseUI.h"
#include "VGUI/ILocalize.h"
#include "vgui.h"
#include "vgui_internal.h"
#include <vgui_controls/controls.h>
#include "vgui_key_translation.h"
#include "KeyValuesSystem.h"
#include <IEngineSurface.h>

IEngineSurface *staticSurface = NULL;

static void (__fastcall *m_pfnBaseUI_Initialize)(void *pthis, int, CreateInterfaceFn *factories, int count) = NULL;
static void (__fastcall *m_pfnBaseUI_Start)(void *pthis, int, struct cl_enginefuncs_s *engineFuncs, int interfaceVersion) = NULL;
static void (__fastcall *m_pfnBaseUI_Shutdown)(void *pthis, int) = NULL;
static int (__fastcall *m_pfnBaseUI_Key_Event)(void *pthis, int, int down, int keynum, const char *pszCurrentBinding) = NULL;
static void (__fastcall *m_pfnBaseUI_CallEngineSurfaceProc)(void *pthis, int, void *hwnd, unsigned int msg, unsigned int wparam, long lparam) = NULL;
static void (__fastcall *m_pfnBaseUI_Paint)(void *pthis, int, int x, int y, int right, int bottom) = NULL;
static void (__fastcall *m_pfnBaseUI_HideGameUI)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnBaseUI_ActivateGameUI)(void *pthis, int) = NULL;
static bool (__fastcall *m_pfnBaseUI_IsGameUIVisible)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnBaseUI_HideConsole)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnBaseUI_ShowConsole)(void *pthis, int) = NULL;

class CBaseUI : public IBaseUI
{
public:
	virtual void Initialize(CreateInterfaceFn *factories, int count);
	virtual void Start(struct cl_enginefuncs_s *engineFuncs, int interfaceVersion);
	virtual void Shutdown(void);
	virtual int Key_Event(int down, int keynum, const char *pszCurrentBinding);
	virtual void CallEngineSurfaceProc(void *hwnd, unsigned int msg, unsigned int wparam, long lparam);
	virtual void Paint(int x, int y, int right, int bottom);
	virtual void HideGameUI(void);
	virtual void ActivateGameUI(void);
	virtual bool IsGameUIVisible(void);
	virtual void HideConsole(void);
	virtual void ShowConsole(void);
};

IBaseUI *g_pBaseUI;
HINTERFACEMODULE g_hVGUI2;

void CBaseUI::Initialize(CreateInterfaceFn *factories, int count)
{
	staticSurface = (IEngineSurface *)factories[0](ENGINE_SURFACE_VERSION, NULL);

	m_pfnBaseUI_Initialize(this, 0, factories, count);
	g_hVGUI2 = (HINTERFACEMODULE)GetModuleHandle("vgui2.dll");
}

void CBaseUI::Start(struct cl_enginefuncs_s *engineFuncs, int interfaceVersion)
{
	m_pfnBaseUI_Start(this, 0, engineFuncs, interfaceVersion);
}

void CBaseUI::Shutdown(void)
{
	m_pfnBaseUI_Shutdown(this, 0);
}

int CBaseUI::Key_Event(int down, int keynum, const char *pszCurrentBinding)
{
	return m_pfnBaseUI_Key_Event(this, 0, down, keynum, pszCurrentBinding);
}

void CBaseUI::CallEngineSurfaceProc(void *hwnd, unsigned int msg, unsigned int wparam, long lparam)
{
	m_pfnBaseUI_CallEngineSurfaceProc(this, 0, hwnd, msg, wparam, lparam);
}

void CBaseUI::Paint(int x, int y, int right, int bottom)
{
	m_pfnBaseUI_Paint(this, 0, x, y, right, bottom);
}

void CBaseUI::HideGameUI(void)
{
	m_pfnBaseUI_HideGameUI(this, 0);
}

void CBaseUI::ActivateGameUI(void)
{
	m_pfnBaseUI_ActivateGameUI(this, 0);
}

bool CBaseUI::IsGameUIVisible(void)
{
	return m_pfnBaseUI_IsGameUIVisible(this, 0);
}

void CBaseUI::HideConsole(void)
{
	m_pfnBaseUI_HideConsole(this, 0);
}

void CBaseUI::ShowConsole(void)
{
	m_pfnBaseUI_ShowConsole(this, 0);
}

void BaseUI_InstallHook(void)
{
	CreateInterfaceFn fnCreateInterface = g_pMetaHookAPI->GetEngineFactory();
	g_pBaseUI = (IBaseUI *)fnCreateInterface(BASEUI_INTERFACE_VERSION, NULL);

	CBaseUI BaseUI;
	DWORD *pVFTable = *(DWORD **)&BaseUI;

	g_pMetaHookAPI->VFTHook(g_pBaseUI, 0, 1, (void *)pVFTable[1], (void *&)m_pfnBaseUI_Initialize);
	g_pMetaHookAPI->VFTHook(g_pBaseUI, 0, 2, (void *)pVFTable[2], (void *&)m_pfnBaseUI_Start);
	g_pMetaHookAPI->VFTHook(g_pBaseUI, 0, 3, (void *)pVFTable[3], (void *&)m_pfnBaseUI_Shutdown);
}