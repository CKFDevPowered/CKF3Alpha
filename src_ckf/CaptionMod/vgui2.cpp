#include <metahook.h>
#include <IEngineSurface.h>
#include "vgui_internal.h"
#include <vstdlib/IKeyValuesSystem.h>
#include <IGameUIFuncs.h>
#include <VGUI/ISystem.h>
#include <VGUI/IPanel.h>
#include <VGUI/IEngineVGui.h>
#include <VGUI/ILocalize.h>
#include "FileSystem.h"
#include "KeyValuesSystem.h"
#include "KeyValues.h"

IEngineSurface *staticSurface;

namespace vgui
{
class IInputInternal *g_pInput;
class ISchemeManager *g_pScheme;
class ISurface *g_pSurface;
class ISystem *g_pSystem;
class IVGui *g_pIVgui;
class IPanel *g_pIPanel;
}

vgui::ILocalize *g_pVGuiLocalize;

IEngineVGui *g_pEngineVGui;
IKeyValuesSystem *g_pKeyValuesSystem;
IFileSystem *g_pFullFileSystem;
IGameUIFuncs *g_pGameUIFuncs;

IKeyValuesSystem *KeyValuesSystem(void)
{
	return g_pKeyValuesSystem;
}

ICommandLine *CommandLine(void)
{
	return g_pInterface->CommandLine;
}

bool vgui::VGui_LoadEngineInterfaces(CreateInterfaceFn vguiFactory, CreateInterfaceFn engineFactory)
{
	staticSurface = (IEngineSurface *)engineFactory(ENGINE_SURFACE_VERSION, NULL);

	g_pKeyValuesSystem = (IKeyValuesSystem *)vguiFactory(KEYVALUESSYSTEM_INTERFACE_VERSION, NULL);
	g_pSystem = (ISystem *)vguiFactory(VGUI_SYSTEM_INTERFACE_VERSION, NULL);
	g_pFullFileSystem = g_pInterface->FileSystem;
	g_pIPanel = (IPanel *)vguiFactory(VGUI_PANEL_INTERFACE_VERSION, NULL);

	g_pKeyValuesSystem->RegisterSizeofKeyValues(sizeof(KeyValues));

	g_pEngineVGui = (IEngineVGui *)engineFactory(VENGINE_VGUI_VERSION, NULL);

	return true;
}