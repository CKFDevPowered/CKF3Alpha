#include <metahook.h>
#include <IBaseUI.h>
#include <IGameUIFuncs.h>
#include <VGUI\IScheme.h>
#include <VGUI\ILocalize.h>
#include <VGUI\IInput.h>
#include "BaseUI.h"
#include "Surface.h"
#include "FontTextureCache.h"
#include <IEngineSurface.h>
#include "vgui_internal.h"
#include "IKeyValuesSystem.h"
#include "KeyValuesSystem.h"
#include "engfuncs.h"

HINTERFACEMODULE g_hVGUI2;
HINTERFACEMODULE g_hGameUI;

extern IEngineSurface *staticSurface;

IBaseUI *g_pBaseUI;
vgui::IPanel *g_pPanel;

void BaseUI_Initalize(CreateInterfaceFn *factories, int count)
{
	g_hVGUI2 = (HINTERFACEMODULE)GetModuleHandle("vgui2.dll");
	if (!g_hVGUI2)
		g_hVGUI2 = (HINTERFACEMODULE)GetModuleHandle("csbte.dll");
	if (g_hVGUI2)
	{
		CreateInterfaceFn fnVGUI2CreateInterface = Sys_GetFactory(g_hVGUI2);
		CreateInterfaceFn fnEngineCreateInterface = g_pMetaHookAPI->GetEngineFactory();

		g_pSurface = (vgui::ISurface *)fnEngineCreateInterface(VGUI_SURFACE_INTERFACE_VERSION, NULL);

		g_pSurface->AddCustomFontFile("resource\\font\\tf2.ttf");
		g_pSurface->AddCustomFontFile("resource\\font\\tf2build.ttf");
		g_pSurface->AddCustomFontFile("resource\\font\\tf2secondary.ttf");

		vgui::VGui_LoadEngineInterfaces(fnVGUI2CreateInterface, fnEngineCreateInterface);

		g_pVGuiLocalize = (ILocalize *)fnVGUI2CreateInterface(VGUI_LOCALIZE_INTERFACE_VERSION, NULL);
		g_pVGuiLocalize->AddFile(g_pFullFileSystem, "resource/ckf3_%language%.txt");
		g_pVGuiLocalize->AddFile(g_pFullFileSystem, "resource/cstrike_%language%.txt");

		g_pGameUIFuncs = (IGameUIFuncs *)factories[0](VENGINE_GAMEUIFUNCS_VERSION, NULL);
	}
}