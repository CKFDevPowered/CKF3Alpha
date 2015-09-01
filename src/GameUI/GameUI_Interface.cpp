#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <io.h>

#include "EngineInterface.h"

#include <tier0/dbg.h>
#include <direct.h>

#include "interface.h"
#include "vgui_controls/controls.h"
#include "GameUI_Interface.h"
#include "vgui_controls/MessageBox.h"
#include "vgui_internal.h"

#include "KeyValues.h"
#include <IGameUIFuncs.h>
#include <IEngineVGui.h>
#include <IBaseUI.h>
#include <IServerBrowser.h>

#include <vgui/ILocalize.h>
#include <vgui/IPanel.h>
#include <vgui/IScheme.h>
#include <vgui/IVGui.h>
#include <vgui/ISystem.h>
#include <vgui/ISurface.h>

#include "BasePanel.h"
#include "ModInfo.h"
#include "GameConsole.h"
#include "LoadingDialog.h"
#include "Configs.h"

cl_enginefunc_t *engine = NULL;

IGameUIFuncs *gameuifuncs = NULL;
IEngineVGui *enginevguifuncs;
vgui::ISurface *enginesurfacefuncs = NULL;
IBaseUI *baseuifuncs = NULL;
IServerBrowser *serverbrowser = NULL;

static CBasePanel *staticPanel = NULL;

static CGameUI g_GameUI;
CGameUI *g_pGameUI = NULL;

vgui::DHANDLE<CLoadingDialog> g_hLoadingDialog;

CGameUI &GameUI(void)
{
	return g_GameUI;
}

vgui::Panel *StaticPanel(void)
{
	return staticPanel;
}

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CGameUI, IGameUI, GAMEUI_INTERFACE_VERSION, g_GameUI);

CGameUI::CGameUI(void)
{
	g_pGameUI = this;
	m_szPreviousStatusText[0] = 0;
	m_bLoadlingLevel = false;
}

CGameUI::~CGameUI(void)
{
	g_pGameUI = NULL;
}

void CGameUI::Initialize(CreateInterfaceFn *factories, int count)
{
	if (!vgui::VGui_InitInterfacesList("GameUI", factories, count))
		return;

	g_pVGuiLocalize->AddFile(g_pFullFileSystem, "Resource/gameui_%language%.txt");
	g_pVGuiLocalize->AddFile(g_pFullFileSystem, "Resource/vgui_%language%.txt");

	enginevguifuncs = (IEngineVGui *)factories[0](VENGINE_VGUI_VERSION, NULL);
	enginesurfacefuncs = (vgui::ISurface *)factories[0](VGUI_SURFACE_INTERFACE_VERSION, NULL);
	gameuifuncs = (IGameUIFuncs *)factories[0](VENGINE_GAMEUIFUNCS_VERSION, NULL);
	baseuifuncs = (IBaseUI *)factories[0](BASEUI_INTERFACE_VERSION, NULL);

	if (!enginesurfacefuncs || !gameuifuncs || !enginevguifuncs || !baseuifuncs)
		Error("CGameUI::Initialize() failed to get necessary interfaces\n");

	serverbrowser = (IServerBrowser *)CreateInterface(SERVERBROWSER_INTERFACE_VERSION, NULL);

	if (serverbrowser)
		serverbrowser->Initialize(factories, count);

	staticPanel = new CBasePanel();
	staticPanel->SetBounds(0, 0, 400, 300);
	staticPanel->SetPaintBorderEnabled(false);
	staticPanel->SetPaintBackgroundEnabled(true);
	staticPanel->SetPaintEnabled(false);
	staticPanel->SetVisible(true);
	staticPanel->SetMouseInputEnabled(false);
	staticPanel->SetKeyBoardInputEnabled(false);
	staticPanel->SetParent(enginevguifuncs->GetPanel(PANEL_GAMEUIDLL));

	if (serverbrowser)
		serverbrowser->SetParent(staticPanel->GetVPanel());

	vgui::surface()->SetAllowHTMLJavaScript(true);
}

void CGameUI::Start(struct cl_enginefuncs_s *engineFuncs, int interfaceVersion, void *system)
{
	engine = engineFuncs;
	engine->pfnClientCmd("cd on\n");
	engine->pfnClientCmd("mp3 loop media/gamestartup.mp3\n");

	ModInfo().LoadCurrentGameInfo();

	if (serverbrowser)
	{
		serverbrowser->ActiveGameName(gConfigs.szGameName, gConfigs.szGameDir);
		serverbrowser->Reactivate();
	}
}

void CGameUI::Shutdown(void)
{
	if (serverbrowser)
	{
		serverbrowser->Deactivate();
		serverbrowser->Shutdown();
	}

	ModInfo().FreeModInfo();
}

int CGameUI::ActivateGameUI(void)
{
	if (!m_bLoadlingLevel && g_hLoadingDialog.Get() && IsInLevel())
	{
		g_hLoadingDialog->Close();
		g_hLoadingDialog = NULL;
	}

	if (!m_bLoadlingLevel)
	{
		if (IsGameUIActive())
			return 1;
	}

	staticPanel->OnGameUIActivated();
	staticPanel->SetVisible(true);

	engine->pfnClientCmd("setpause");
	return 1;
}

int CGameUI::ActivateDemoUI(void)
{
	staticPanel->OnOpenDemoDialog();
	return 1;
}

int CGameUI::HasExclusiveInput(void)
{
	return IsGameUIActive();
}

void CGameUI::RunFrame(void)
{
	int wide, tall;
	vgui::surface()->GetScreenSize(wide, tall);
	staticPanel->SetSize(wide, tall);

	BasePanel()->RunFrame();
}

void CGameUI::ConnectToServer(const char *game, int IP, int port)
{
	if (serverbrowser)
		serverbrowser->ConnectToGame(IP, port);

	engine->pfnClientCmd("mp3 stop\n");

	baseuifuncs->HideGameUI();
}

void CGameUI::DisconnectFromServer(void)
{
	if (serverbrowser)
		serverbrowser->DisconnectFromGame();

	baseuifuncs->ActivateGameUI();
}

void CGameUI::HideGameUI(void)
{
	if (!IsGameUIActive())
		return;

	if (!IsInLevel())
		return;

	staticPanel->SetVisible(false);

	engine->pfnClientCmd("unpause");
	engine->pfnClientCmd("hideconsole");

	if (!m_bLoadlingLevel && g_hLoadingDialog.Get())
	{
		g_hLoadingDialog->Close();
		g_hLoadingDialog = NULL;
	}
}

bool CGameUI::IsGameUIActive(void)
{
	return staticPanel->IsVisible();
}

void CGameUI::LoadingStarted(const char *resourceType, const char *resourceName)
{
	m_bLoadlingLevel = true;

	staticPanel->OnLevelLoadingStarted(resourceName);
}

void CGameUI::LoadingFinished(const char *resourceType, const char *resourceName)
{
	m_bLoadlingLevel = false;

	staticPanel->OnLevelLoadingFinished();
	baseuifuncs->HideGameUI();
}

void CGameUI::StartProgressBar(const char *progressType, int progressSteps)
{
	if (!g_hLoadingDialog.Get())
		g_hLoadingDialog = new CLoadingDialog(staticPanel);

	m_szPreviousStatusText[0] = 0;
	g_hLoadingDialog->SetProgressRange(0, progressSteps);
	g_hLoadingDialog->SetProgressPoint(0.0f);
	g_hLoadingDialog->Open();
}

int CGameUI::ContinueProgressBar(int progressPoint, float progressFraction)
{
	if (!g_hLoadingDialog.Get())
	{
		g_hLoadingDialog = new CLoadingDialog(staticPanel);
		g_hLoadingDialog->SetProgressRange(0, 24);
		g_hLoadingDialog->SetProgressPoint(0.0f);
		g_hLoadingDialog->Open();
	}

	g_hLoadingDialog->Activate();
	g_hLoadingDialog->SetProgressPoint(progressPoint);
	return 1;
}

void CGameUI::StopProgressBar(bool bError, const char *failureReason, const char *extendedReason)
{
	if (!g_hLoadingDialog.Get() && bError)
		g_hLoadingDialog = new CLoadingDialog(staticPanel);

	if (!g_hLoadingDialog.Get())
		return;

	if (bError)
	{
		g_hLoadingDialog->DisplayGenericError(failureReason, extendedReason);
	}
	else
	{
		g_hLoadingDialog->Close();
		g_hLoadingDialog = NULL;
	}
}

int CGameUI::SetProgressBarStatusText(const char *statusText)
{
	if (!g_hLoadingDialog.Get())
		return false;

	if (!statusText)
		return false;

	if (!stricmp(statusText, m_szPreviousStatusText))
		return false;

	g_hLoadingDialog->SetStatusText(statusText);
	Q_strncpy(m_szPreviousStatusText, statusText, sizeof(m_szPreviousStatusText));
	return true;
}

void CGameUI::SetSecondaryProgressBar(float progress)
{
	if (!g_hLoadingDialog.Get())
		return;

	g_hLoadingDialog->SetSecondaryProgress(progress);
}

void CGameUI::SetSecondaryProgressBarText(const char *statusText)
{
	if (!g_hLoadingDialog.Get())
		return;

	g_hLoadingDialog->SetSecondaryProgressText(statusText);
}

void CGameUI::ValidateCDKey(bool force, bool inConnect)
{
}

bool CGameUI::IsInLevel(void)
{
	const char *levelName = engine->pfnGetLevelName();

	if (strlen(levelName) > 0)
		return true;

	return false;
}

bool CGameUI::IsInMultiplayer(void)
{
	return (IsInLevel() && engine->GetMaxClients() > 1);
}