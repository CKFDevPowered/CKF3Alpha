#include "EngineInterface.h"
#include "BasePanel.h"

#include "vgui/IInputInternal.h"
#include "vgui/ILocalize.h"
#include "vgui/IPanel.h"
#include "vgui/ISurface.h"
#include "vgui/ISystem.h"
#include "vgui/IVGui.h"
#include "FileSystem.h"
#include "GameConsole.h"
#include "GameUI_Interface.h"

#include "IGameuiFuncs.h"
#include "IServerBrowser.h"
#include "ICommandLine.h"

#include "vgui_controls/AnimationController.h"
#include "vgui_controls/ImagePanel.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/Menu.h"
#include "vgui_controls/MenuItem.h"
#include "vgui_controls/PHandle.h"
#include "vgui_controls/MessageBox.h"
#include "vgui_controls/QueryBox.h"

#include "tier1/fmtstr.h"

#include "ModInfo.h"
#include "LoadingDialog.h"
#include "BackgroundMenuButton.h"
#include "OptionsDialog.h"
#include "CreateMultiplayerGameDialog.h"
#include "GameMenu.h"

using namespace vgui;

extern vgui::DHANDLE<CLoadingDialog> g_hLoadingDialog;
extern CBasePanel *staticPanel;
extern IServerBrowser *serverbrowser;

static CBasePanel *g_pBasePanel = NULL;
static float g_flAnimationPadding = 0.01f;

CBasePanel *BasePanel(void)
{
	return g_pBasePanel;
}

VPANEL GetGameUIBasePanel(void)
{
	return BasePanel()->GetVPanel();
}

CBasePanel::CBasePanel(void) : Panel(NULL, "BaseGameUIPanel")
{
	g_pBasePanel = this;

	m_bLevelLoading = false;
	m_eBackgroundState = BACKGROUND_INITIAL;
	m_flTransitionStartTime = 0.0f;
	m_flTransitionEndTime = 0.0f;
	m_flFrameFadeInTime = 0.5f;
	m_bRenderingBackgroundTransition = false;
	m_bFadingInMenus = false;
	m_bEverActivated = false;
	m_iGameMenuInset = 24;
	m_bHaveDarkenedBackground = false;
	m_bHaveDarkenedTitleText = true;
	m_bForceTitleTextUpdate = true;

	m_BackdropColor = Color(0, 0, 0, 128);
	m_ExitingFrameCount = 0;

	m_pGameMenuButton = new CBackgroundMenuButton(this, "GameMenuButton");
	m_pGameMenu = NULL;

	CreateGameMenu();
	SetMenuAlpha(0);
}

CBasePanel::~CBasePanel(void)
{
	g_pBasePanel = NULL;
}

void CBasePanel::PaintBackground(void)
{
	if (!m_hOptionsDialog.Get())
	{
		m_hOptionsDialog = new COptionsDialog(this);
		m_hCreateMultiplayerGameDialog = new CCreateMultiplayerGameDialog(this);

		PositionDialog(m_hOptionsDialog);
		PositionDialog(m_hCreateMultiplayerGameDialog);
	}

	if (!GameUI().IsInLevel() || g_hLoadingDialog.Get())
	{
		DrawBackgroundImage();
	}

	if (m_flBackgroundFillAlpha)
	{
		int swide, stall;
		surface()->GetScreenSize(swide, stall);
		surface()->DrawSetColor(0, 0, 0, m_flBackgroundFillAlpha);
		surface()->DrawFilledRect(0, 0, swide, stall);
	}
}

void CBasePanel::UpdateBackgroundState(void)
{
	if (m_ExitingFrameCount)
	{
		SetBackgroundRenderState(BACKGROUND_EXITING);
	}
	else if (GameUI().IsInLevel())
	{
		SetBackgroundRenderState(BACKGROUND_LEVEL);
	}
	else if (!m_bLevelLoading)
	{
		if (IsPC() || (IsX360() && !g_hLoadingDialog.Get()))
			SetBackgroundRenderState(BACKGROUND_MAINMENU);
	}
	else if (m_bLevelLoading && g_hLoadingDialog.Get())
	{
		SetBackgroundRenderState(BACKGROUND_LOADING);
	}
	else if (m_bEverActivated)
	{
		SetBackgroundRenderState(BACKGROUND_DISCONNECTED);
	}

	bool bHaveActiveDialogs = false;
	bool bIsInLevel = GameUI().IsInLevel();

	for (int i = 0; i < GetChildCount(); ++i)
	{
		VPANEL child = ipanel()->GetChild(GetVPanel(), i);
		const char *name = ipanel()->GetName(child);

		if (child && ipanel()->IsVisible(child) && ipanel()->IsPopup(child) && child != m_pGameMenu->GetVPanel())
		{
			bHaveActiveDialogs = true;
			break;
		}
	}

	if (!bHaveActiveDialogs)
	{
		VPANEL parent = GetVParent();

		for (int i = 0; i < ipanel()->GetChildCount(parent); ++i)
		{
			VPANEL child = ipanel()->GetChild(parent, i);

			if (child && ipanel()->IsVisible(child) && ipanel()->IsPopup(child) && child != GetVPanel())
			{
				bHaveActiveDialogs = true;
				break;
			}
		}
	}

	bool bNeedDarkenedBackground = (bHaveActiveDialogs || bIsInLevel);

	if (m_bHaveDarkenedBackground != bNeedDarkenedBackground)
	{
		float targetAlpha, duration;

		if (bNeedDarkenedBackground || m_eBackgroundState == BACKGROUND_LOADING)
		{
			targetAlpha = m_BackdropColor[3];
			duration = m_flFrameFadeInTime;
		}
		else
		{
			targetAlpha = 0.0f;
			duration = 2.0f;
		}

		GetAnimationController()->RunAnimationCommand(this, "m_flBackgroundFillAlpha", targetAlpha, 0.0f, duration, AnimationController::INTERPOLATOR_LINEAR);

		m_bHaveDarkenedBackground = bNeedDarkenedBackground;
	}

	if (m_bLevelLoading)
		return;

	bool bNeedDarkenedTitleText = bHaveActiveDialogs;

	if (m_bHaveDarkenedTitleText != bNeedDarkenedTitleText || m_bForceTitleTextUpdate)
	{
		float targetTitleAlpha, duration;

		if (bHaveActiveDialogs || m_eBackgroundState == BACKGROUND_LOADING)
		{
			duration = m_flFrameFadeInTime;
			targetTitleAlpha = 128.0f;
		}
		else
		{
			duration = 2.0f;
			targetTitleAlpha = 255.0f;
		}

		if (m_pGameMenu)
			GetAnimationController()->RunAnimationCommand(m_pGameMenu, "alpha", targetTitleAlpha, 0.0f, duration, AnimationController::INTERPOLATOR_LINEAR);

		if (m_pGameMenuButton)
			GetAnimationController()->RunAnimationCommand(m_pGameMenuButton, "alpha", targetTitleAlpha, 0.0f, duration, AnimationController::INTERPOLATOR_LINEAR);

		m_bFadingInMenus = false;
		m_bHaveDarkenedTitleText = bNeedDarkenedTitleText;
		m_bForceTitleTextUpdate = false;
	}
}

void CBasePanel::SetBackgroundRenderState(EBackgroundState state)
{
	if (state == m_eBackgroundState)
		return;

	float frametime = system()->GetCurrentTime();

	m_bRenderingBackgroundTransition = false;
	m_bFadingInMenus = false;

	if (state == BACKGROUND_EXITING)
	{
	}
	else if (state == BACKGROUND_DISCONNECTED || state == BACKGROUND_MAINMENU)
	{
		m_bFadingInMenus = true;
		m_flFadeMenuStartTime = frametime;
		m_flFadeMenuEndTime = frametime + 3.0f;
	}
	else if (state == BACKGROUND_LOADING)
	{
		SetMenuAlpha(0);
	}
	else if (state == BACKGROUND_LEVEL)
	{
		SetMenuAlpha(255);
	}

	m_eBackgroundState = state;
}

void CBasePanel::StartExitingProcess(void)
{
	m_flTransitionStartTime = system()->GetCurrentTime();
	m_flTransitionEndTime = m_flTransitionStartTime + 0.5f;
	m_ExitingFrameCount = 30;

	engine->pfnClientCmd("quit\n");

	SetVisible(false);
	vgui::surface()->RestrictPaintToSinglePanel(GetVPanel());
}

void CBasePanel::OnSizeChanged(int newWide, int newTall)
{
}

void CBasePanel::OnLevelLoadingStarted(const char *levelName)
{
	m_bLevelLoading = true;

	if (!g_hLoadingDialog.Get())
		g_hLoadingDialog = new CLoadingDialog(this);
}

void CBasePanel::OnLevelLoadingFinished(void)
{
	m_bLevelLoading = false;
}

void CBasePanel::DrawBackgroundImage(void)
{
	int swide, stall;
	surface()->GetScreenSize(swide, stall);

	int wide, tall;
	GetSize(wide, tall);

	float frametime = system()->GetCurrentTime();
	int alpha = 255;

	if (m_bRenderingBackgroundTransition)
	{
		alpha = (m_flTransitionEndTime - frametime) / (m_flTransitionEndTime - m_flTransitionStartTime) * 255;
		alpha = clamp(alpha, 0, 255);
	}

	if (m_ExitingFrameCount)
	{
		alpha = (m_flTransitionEndTime - frametime) / (m_flTransitionEndTime - m_flTransitionStartTime) * 255;
		alpha = 255 - clamp(alpha, 0, 255);
	}

	if (IsPC() && (m_bRenderingBackgroundTransition || (m_eBackgroundState == BACKGROUND_LOADING || m_eBackgroundState == BACKGROUND_MAINMENU)))
	{
		float xScale = swide / 800.0f;
		float yScale = stall / 600.0f;

		int ypos = 0;

		for (int y = 0; y < BACKGROUND_ROWS; y++)
		{
			int xpos = 0;

			for (int x = 0; x < BACKGROUND_COLUMNS; x++)
			{
				bimage_t &bimage = m_ImageID[y][x];

				int dx = (int)ceil(xpos * xScale);
				int dy = (int)ceil(ypos * yScale);
				int dw = (int)ceil((xpos + bimage.width) * xScale);
				int dt = (int)ceil((ypos + bimage.height) * yScale);

				if (x == 0)
					dx = 0;

				if (y == 0)
					dy = 0;

				surface()->DrawSetColor(255, 255, 255, alpha);
				surface()->DrawSetTexture(bimage.imageID);
				surface()->DrawTexturedRect(dx, dy, dw, dt);

				xpos += bimage.width;
			}

			ypos += m_ImageID[y][0].height;
		}

		if (m_eBackgroundState == BACKGROUND_LOADING)
		{
			surface()->DrawSetColor(255, 255, 255, alpha);
			surface()->DrawSetTexture(m_iLoadingImageID);

			int twide, ttall;
			surface()->DrawGetTextureSize(m_iLoadingImageID, twide, ttall);
			surface()->DrawTexturedRect(wide - twide, tall - ttall, wide, tall);
		}
	}

	if (m_bFadingInMenus)
	{
		alpha = (frametime - m_flFadeMenuStartTime) / (m_flFadeMenuEndTime - m_flFadeMenuStartTime) * 255;
		alpha = clamp(alpha, 0, 255);

		if (alpha == 255)
			m_bFadingInMenus = false;

		m_pGameMenuButton->SetAlpha(alpha);
		m_pGameMenu->SetAlpha(alpha);
	}
}

void CBasePanel::CreateGameMenu(void)
{
	KeyValues *datafile = new KeyValues("GameMenu");
	datafile->UsesEscapeSequences(true);

	if (datafile->LoadFromFile(g_pFullFileSystem, "Resource/GameMenu.res"))
	{
		m_pGameMenu = RecursiveLoadGameMenu(datafile);
		SETUP_PANEL(m_pGameMenu);
		m_pGameMenu->SetAlpha(0);

		datafile->deleteThis();
	}
	else
	{
		Error("Could not load file Resource/GameMenu.res");
	}
}

void CBasePanel::UpdateGameMenus(void)
{
	bool isInGame = GameUI().IsInLevel();
	bool isMulti = isInGame && (engine->GetMaxClients() > 1);

	m_pGameMenu->UpdateMenuItemState(isInGame, isMulti);
	m_pGameMenu->SetVisible(true);

	InvalidateLayout();
}

CGameMenu *CBasePanel::RecursiveLoadGameMenu(KeyValues *datafile)
{
	CGameMenu *menu = new CGameMenu(this, datafile->GetName());

	for (KeyValues *dat = datafile->GetFirstSubKey(); dat != NULL; dat = dat->GetNextKey())
	{
		const char *label = dat->GetString("label", "<unknown>");
		const char *cmd = dat->GetString("command", NULL);
		const char *name = dat->GetString("name", label);

		if (!Q_stricmp(cmd, "OpenServerBrowser"))
		{
			if (!serverbrowser)
				continue;
		}

		menu->AddMenuItem(name, label, cmd, this, dat);
	}

	return menu;
}

void CBasePanel::RunFrame(void)
{
	GameConsole().SetParent(GetVPanel());

	if (m_ExitingFrameCount > 1 && system()->GetCurrentTime() >= m_flTransitionEndTime)
	{
		m_ExitingFrameCount--;

		if (m_ExitingFrameCount == 1)
		{
			engine->pfnClientCmd("quit\n");
			return;
		}
	}

	GetAnimationController()->UpdateAnimations(system()->GetCurrentTime());

	if (IsVisible())
		surface()->PaintTraverse(GetVPanel());

	if (!m_ExitingFrameCount)
	{
		UpdateBackgroundState();
		UpdateGameMenus();
	}
}

void CBasePanel::PerformLayout(void)
{
	BaseClass::PerformLayout();

	int wide, tall;
	vgui::surface()->GetScreenSize(wide, tall);

	int menuButtonWide, menuButtonTall;
	m_pGameMenuButton->GetSize(menuButtonWide, menuButtonTall);
	m_pGameMenuButton->SetPos(m_iGameMenuInset, tall - menuButtonTall - (m_iGameMenuInset) / 2);

	int menuWide, menuTall;
	m_pGameMenu->GetSize(menuWide, menuTall);
	m_pGameMenu->SetPos(m_iGameMenuInset, tall - menuTall - menuButtonTall - m_iGameMenuInset);
}

void CBasePanel::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_iGameMenuInset = atoi(pScheme->GetResourceString("MainMenu.Inset"));

	m_pGameMenuButton->SetImages(scheme()->GetImage("resource/game_menu", false), scheme()->GetImage("resource/game_menu_mouseover", false));

	m_flFrameFadeInTime = atof(pScheme->GetResourceString("Frame.TransitionEffectTime"));
	m_BackdropColor = pScheme->GetColor("mainmenu.backdrop", Color(0, 0, 0, 128));

	for (int y = 0; y < BACKGROUND_ROWS; y++)
	{
		for (int x = 0; x < BACKGROUND_COLUMNS; x++)
		{
			bimage_t &bimage = m_ImageID[y][x];
			bimage.imageID = surface()->CreateNewTextureID();

			char filename[MAX_PATH];
			sprintf(filename, "resource/background/800_%d_%c_loading", y + 1, 'a' + x);
			surface()->DrawSetTextureFile(bimage.imageID, filename, false, false);
			surface()->DrawGetTextureSize(bimage.imageID, bimage.width, bimage.height);
		}
	}

	if (IsPC())
	{
		m_iLoadingImageID = surface()->CreateNewTextureID();
		surface()->DrawSetTextureFile(m_iLoadingImageID, "gfx/vgui/console/startup_loading", false, false);
	}
}

void CBasePanel::OnActivateModule(int moduleIndex)
{
}

void CBasePanel::OnGameUIActivated(void)
{
	if (!m_bEverActivated)
	{
		UpdateGameMenus();
		m_bEverActivated = true;
	}

	if (GameUI().IsInLevel())
		OnCommand("OpenPauseMenu");
}

void CBasePanel::RunMenuCommand(const char *command)
{
	if (!Q_stricmp(command, "OpenServerBrowser"))
	{
		OnOpenServerBrowser();
	}
	else if (!Q_stricmp(command, "OpenCreateMultiplayerGameDialog"))
	{
		OnOpenCreateMultiplayerGameDialog();
	}
	else if (!Q_stricmp(command, "OpenOptionsDialog"))
	{
		OnOpenOptionsDialog();
	}
	else if (!Q_stricmp(command, "ResumeGame"))
	{
		engine->pfnClientCmd("cancelselect");
	}
	else if (!Q_stricmp(command, "Disconnect"))
	{
		engine->pfnClientCmd("disconnect");
	}
	else if (!Q_stricmp(command, "Quit"))
	{
		OnOpenQuitConfirmationDialog();
	}
	else if (!Q_stricmp(command, "QuitNoConfirm"))
	{
		StartExitingProcess();
	}
	else if (!Q_stricmp(command, "ReleaseModalWindow"))
	{
		vgui::surface()->RestrictPaintToSinglePanel(NULL);
	}
	else if (strstr(command, "engine "))
	{
		const char *engineCMD = strstr(command, "engine ") + strlen("engine ");

		if (strlen(engineCMD) > 0)
		{
			engine->pfnClientCmd(const_cast<char *>(engineCMD));
		}

		engine->pfnClientCmd("cancelselect");
	}
	else
	{
		BaseClass::OnCommand(command);
	}
}

void CBasePanel::OnCommand(const char *command)
{
	RunMenuCommand(command);
}

void CBasePanel::OnOpenQuitConfirmationDialog(void)
{
	QueryBox *box = new QueryBox("#GameUI_QuitConfirmationTitle", "#GameUI_QuitConfirmationText", this);
	box->SetOKButtonText("#GameUI_Quit");
	box->SetOKCommand(new KeyValues("Command", "command", "QuitNoConfirm"));
	box->SetCancelCommand(new KeyValues("Command", "command", "ReleaseModalWindow"));
	box->AddActionSignalTarget(this);
	box->DoModal();

	vgui::surface()->RestrictPaintToSinglePanel(box->GetVPanel());
}

void CBasePanel::OnOpenServerBrowser(void)
{
	if (serverbrowser)
		serverbrowser->Activate();
}

void CBasePanel::OnOpenOptionsDialog(void)
{
	if (!m_hOptionsDialog.Get())
	{
		m_hOptionsDialog = new COptionsDialog(this);
		PositionDialog(m_hOptionsDialog);
	}

	m_hOptionsDialog->Activate();
}

void CBasePanel::OnOpenCreateMultiplayerGameDialog(void)
{
	if (!m_hCreateMultiplayerGameDialog.Get())
	{
		m_hCreateMultiplayerGameDialog = new CCreateMultiplayerGameDialog(this);
		PositionDialog(m_hCreateMultiplayerGameDialog);
	}

	m_hCreateMultiplayerGameDialog->Activate();
}

void CBasePanel::OnOpenDemoDialog(void)
{
}

void CBasePanel::PositionDialog(vgui::PHandle dlg)
{
	if (!dlg.Get())
		return;

	int x, y, ww, wt, wide, tall;
	vgui::surface()->GetWorkspaceBounds(x, y, ww, wt);
	dlg->GetSize(wide, tall);
	dlg->SetPos(x + ((ww - wide) / 2), y + ((wt - tall) / 2));
}

void CBasePanel::OnGameUIHidden(void)
{
	if (m_hOptionsDialog.Get())
		PostMessage(m_hOptionsDialog.Get(), new KeyValues("GameUIHidden"));
}

void CBasePanel::SetMenuAlpha(int alpha)
{
	m_pGameMenu->SetAlpha(alpha);
	m_pGameMenuButton->SetAlpha(alpha);

	m_bForceTitleTextUpdate = true;
}

void CBasePanel::SetMenuItemBlinkingState(const char *itemName, bool state)
{
	for (int i = 0; i < GetChildCount(); i++)
	{
		Panel *child = GetChild(i);
		CGameMenu *pGameMenu = dynamic_cast<CGameMenu *>(child);

		if (pGameMenu)
			pGameMenu->SetMenuItemBlinkingState(itemName, state);
	}
}

void CBasePanel::RunEngineCommand(const char *command)
{
	engine->pfnClientCmd((char *)command);
}

void CBasePanel::FinishDialogClose(void)
{
}