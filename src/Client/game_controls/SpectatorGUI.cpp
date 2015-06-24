#include "hud.h"
#include "SpectatorGUI.h"
#include "ClientScoreBoardDialog.h"
#include "commandmenu.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <FileSystem.h>
#include <IGameUIFuncs.h>

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/TextImage.h>
#include <vgui_controls/ImagePanel.h>

#include "CounterStrikeViewport.h"

#include "cl_entity.h"
#include "pm_shared.h"
#include "cvardef.h"

extern IGameUIFuncs *gameuifuncs;

CSpectatorGUI *g_pSpectatorGUI = NULL;
CSpectatorMenu *g_pSpectatorMenu = NULL;

static char *s_SpectatorModes[] = { "#Spec_Mode0", "#Spec_Mode1", "#Spec_Mode2", "#Spec_Mode3", "#Spec_Mode4", "#Spec_Mode5", "" };

using namespace vgui;

class CSpecButton : public Button
{
public:
	CSpecButton(Panel *parent, const char *panelName): Button(parent, panelName, "") {}

private:
	void ApplySchemeSettings(vgui::IScheme *pScheme)
	{
		Button::ApplySchemeSettings(pScheme);
		SetFont(pScheme->GetFont("Marlett", IsProportional()));
	}
};

CSpectatorMenu::CSpectatorMenu(void) : Frame(NULL, PANEL_SPECMENU)
{
	g_pSpectatorMenu = this;

	SetMouseInputEnabled(true);
	SetKeyBoardInputEnabled(true);
	SetTitleBarVisible(false);
	SetMoveable(false);
	SetSizeable(false);
	SetProportional(true);

	SetScheme("ClientScheme");

	m_iDuckKey = KEY_NONE;
	m_pPlayerList = new ComboBox(this, "playercombo", 10, false);

	HFont hFallbackFont = scheme()->GetIScheme(GetScheme())->GetFont("DefaultVerySmallFallBack", false);

	if (INVALID_FONT != hFallbackFont)
		m_pPlayerList->SetUseFallbackFont(true, hFallbackFont);

	m_pViewOptions = new ComboBox(this, "viewcombo", 10, false);
	m_pConfigSettings = new ComboBox(this, "settingscombo", 10, false);

	m_pLeftButton = new CSpecButton(this, "specprev");
	m_pLeftButton->SetText("3");
	m_pRightButton = new CSpecButton(this, "specnext");
	m_pRightButton->SetText("4");

	m_pPlayerList->SetText("");
	m_pViewOptions->SetText("#Spec_Modes");
	m_pConfigSettings->SetText("#Spec_Options");

	m_pPlayerList->SetOpenDirection(Menu::UP);
	m_pViewOptions->SetOpenDirection(Menu::UP);
	m_pConfigSettings->SetOpenDirection(Menu::UP);

	m_pSpectatorMenu = new CommandMenu(m_pViewOptions, "spectatormenu");
	m_pSpectatorMenu->LoadFromFile("Resource/SpectatorMenu.res");
	m_pConfigSettings->SetMenu(m_pSpectatorMenu);

	m_pSpectatorModes = new CommandMenu(m_pViewOptions, "spectatormodes");
	m_pSpectatorModes->LoadFromFile("Resource/SpectatorModes.res");
	m_pViewOptions->SetMenu(m_pSpectatorModes);

	LoadControlSettings("Resource/UI/BottomSpectator.res", "GAME");
}

void CSpectatorMenu::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_pConfigSettings->GetMenu()->MakeReadyForUse();
	m_pViewOptions->GetMenu()->MakeReadyForUse();
	m_pPlayerList->GetMenu()->MakeReadyForUse();

	if (g_pSpectatorGUI)
	{
		m_pConfigSettings->GetMenu()->SetBgColor(g_pSpectatorGUI->GetBlackBarColor());
		m_pViewOptions->GetMenu()->SetBgColor(g_pSpectatorGUI->GetBlackBarColor());
		m_pPlayerList->GetMenu()->SetBgColor(g_pSpectatorGUI->GetBlackBarColor());
	}

	m_pSpectatorMenu->UpdateMenu();
	m_pSpectatorModes->UpdateMenu();

	m_pConfigSettings->InvalidateLayout(true);
	m_pViewOptions->InvalidateLayout(true);
	m_pPlayerList->InvalidateLayout(true);

	m_pConfigSettings->GetMenu()->InvalidateLayout(true);
	m_pViewOptions->GetMenu()->InvalidateLayout(true);
	m_pPlayerList->GetMenu()->InvalidateLayout(true);
}

void CSpectatorMenu::PerformLayout(void)
{
	int w, h;
	surface()->GetScreenSize(w, h);
	SetSize(w, GetTall());
}

void CSpectatorMenu::OnTextChanged(KeyValues *data)
{
	Panel *panel = reinterpret_cast<vgui::Panel *>(data->GetPtr("panel"));
	vgui::ComboBox *box = dynamic_cast<vgui::ComboBox *>(panel);

	if (box == m_pConfigSettings)
	{
		m_pConfigSettings->SetText("#Spec_Options");
	}
	else if (box == m_pPlayerList)
	{
		KeyValues *kv = box->GetActiveItemUserData();

		if (kv)
		{
			int currentPlayerNum = g_iUser2;

			if (g_iUser2)
			{
				const char *player = kv->GetString("player");
				const char *currentPlayerName = g_PlayerInfoList[currentPlayerNum].name;

				if (Q_strcmp(currentPlayerName, player))
				{
					char command[128];
					Q_snprintf(command, sizeof(command), "follow \"%s\"", player);
					gEngfuncs.pfnServerCmd(command);
				}
			}
		}
	}
}

void CSpectatorMenu::OnCommand(const char *command)
{
	if (!stricmp(command, "specnext"))
	{
		gEngfuncs.pfnServerCmd("follownext 0");
	}
	else if (!stricmp(command, "specprev"))
	{
		gEngfuncs.pfnServerCmd("follownext 1");
	}
}

void CSpectatorMenu::OnKeyCodePressed(KeyCode code)
{
	if (code == m_iDuckKey)
	{
		if (!m_bDuckPressed)
			ShowPanel(false);
	}
	else
		BaseClass::OnKeyCodePressed(code);
}

void CSpectatorMenu::OnKeyCodeReleased(vgui::KeyCode code)
{
	if (code == m_iDuckKey)
	{
		m_bDuckPressed = false;
	}
	else
		BaseClass::OnKeyCodeReleased(code);
}

void CSpectatorMenu::ShowPanel(bool bShow)
{
	if (bShow)
	{
		m_pConfigSettings->HideMenu();
		m_pViewOptions->HideMenu();
		m_pPlayerList->HideMenu();

		if (g_pViewPort->IsScoreBoardVisible())
			g_pViewPort->ShowPanel(PANEL_SCOREBOARD, false);

		Activate();

		m_bDuckPressed = input()->WasKeyPressed(m_iDuckKey);
		m_pPlayerList->RequestFocus();

		if (g_pSpectatorGUI)
			g_pSpectatorGUI->Update();
	}
	else
	{
		SetVisible(false);
	}
}

bool IsConnected(int playerIndex);

void CSpectatorMenu::Update(void)
{
	wchar_t playerText[80], playerName[64], *team, teamText[64];
	char localizeTeamName[64];
	char szPlayerIndex[16];

	m_pPlayerList->DeleteAllItems();

	if (m_iDuckKey == KEY_NONE)
		m_iDuckKey = gameuifuncs->GetVGUI2KeyCodeForBind("duck");

	if (g_iUser2 && g_iUser2 != gHUD.m_iPlayerNum)
	{
		for (int iPlayerIndex = 1; iPlayerIndex <= gEngfuncs.GetMaxClients(); iPlayerIndex++)
		{
			if (!IsConnected(iPlayerIndex))
				continue;

			if (iPlayerIndex == gHUD.m_iPlayerNum)
				continue;

			if (g_PlayerExtraInfo[iPlayerIndex].dead)
				continue;

			const char *oldName = g_PlayerInfoList[iPlayerIndex].name;
			int bufsize = strlen(oldName) * 2 + 1;
			char *newName = (char *)_alloca(bufsize);
			gViewPortInterface->MakeSafeName(oldName, newName, bufsize);
			g_pVGuiLocalize->ConvertANSIToUnicode(newName, playerName, sizeof(playerName));

			const char *teamname = g_pViewPort->GetTeamName(g_PlayerExtraInfo[iPlayerIndex].teamnumber);

			if (teamname)
			{
				Q_snprintf(localizeTeamName, sizeof(localizeTeamName), "#%s", teamname);
				team = g_pVGuiLocalize->Find(localizeTeamName);

				if (!team)
				{
					g_pVGuiLocalize->ConvertANSIToUnicode(teamname, teamText, sizeof(teamText));
					team = teamText;
				}

				g_pVGuiLocalize->ConstructString(playerText, sizeof(playerText), g_pVGuiLocalize->Find("#Spec_PlayerItem_Team"), 2, playerName, team);
			}
			else
			{
				g_pVGuiLocalize->ConstructString(playerText, sizeof(playerText), g_pVGuiLocalize->Find("#Spec_PlayerItem"), 1, playerName);
			}

			Q_snprintf(szPlayerIndex, sizeof(szPlayerIndex), "%d", iPlayerIndex);

			KeyValues *kv = new KeyValues("UserData", "player", g_PlayerInfoList[iPlayerIndex].name, "index", szPlayerIndex);
			m_pPlayerList->AddItem(playerText, kv);
			kv->deleteThis();
		}

		int playernum = g_iUser2;
		const char *selectedPlayerName = g_PlayerInfoList[playernum].name;

		for (int iPlayerIndex = 0; iPlayerIndex < m_pPlayerList->GetItemCount(); ++iPlayerIndex)
		{
			KeyValues *kv = m_pPlayerList->GetItemUserData(iPlayerIndex);

			if (kv && selectedPlayerName && !Q_strcmp(kv->GetString("player"), selectedPlayerName))
			{
				m_pPlayerList->ActivateItemByRow(iPlayerIndex);
				break;
			}
		}
	}
	else
	{
		int iPlayerIndex = gHUD.m_iPlayerNum;
		const char *oldName = g_PlayerInfoList[iPlayerIndex].name;
		int bufsize = strlen(oldName) * 2 + 1;
		char *newName = (char *)_alloca(bufsize);
		gViewPortInterface->MakeSafeName(oldName, newName, bufsize);
		g_pVGuiLocalize->ConvertANSIToUnicode(newName, playerName, sizeof(playerName));

		const char *teamname = g_pViewPort->GetTeamName(g_PlayerExtraInfo[iPlayerIndex].teamnumber);

		if (teamname)
		{
			Q_snprintf(localizeTeamName, sizeof(localizeTeamName), "#%s", teamname);
			team = g_pVGuiLocalize->Find(localizeTeamName);

			if (!team)
			{
				g_pVGuiLocalize->ConvertANSIToUnicode(teamname, teamText, sizeof(teamText));
				team = teamText;
			}

			g_pVGuiLocalize->ConstructString(playerText, sizeof(playerText), g_pVGuiLocalize->Find("#Spec_PlayerItem_Team"), 2, playerName, team);
		}
		else
		{
			g_pVGuiLocalize->ConstructString(playerText, sizeof(playerText), g_pVGuiLocalize->Find("#Spec_PlayerItem"), 1, playerName);
		}

		Q_snprintf(szPlayerIndex, sizeof(szPlayerIndex), "%d", iPlayerIndex);

		KeyValues *kv = new KeyValues("UserData", "player", g_PlayerInfoList[iPlayerIndex].name, "index", szPlayerIndex);
		m_pPlayerList->AddItem(playerText, kv);
		m_pPlayerList->ActivateItemByRow(0);
		kv->deleteThis();
	}
}

CSpectatorGUI::CSpectatorGUI(void) : EditablePanel(NULL, PANEL_SPECGUI)
{
	m_bSpecScoreboard = false;
	m_bHelpShown = false;

	g_pSpectatorGUI = this;

	SetVisible(false);
	SetProportional(true);
	SetCursor(dc_none);

	SetScheme("ClientScheme");
	SetMouseInputEnabled(false);
	SetKeyBoardInputEnabled(false);

	m_pTopBar = new Panel(this, "topbar");
	m_pBottomBarBlank = new Panel(this, "bottombarblank");

	m_pPlayerLabel = new Label(this, "playerlabel", "");
	m_pPlayerLabel->SetVisible(false);

	TextImage *image = m_pPlayerLabel->GetTextImage();

	if (image)
	{
		HFont hFallbackFont = scheme()->GetIScheme(GetScheme())->GetFont("DefaultVerySmallFallBack", false);

		if (INVALID_FONT != hFallbackFont)
			image->SetUseFallbackFont(true, hFallbackFont);
	}

	SetPaintBorderEnabled(false);
	SetPaintBackgroundEnabled(false);

	LoadControlSettings("Resource/UI/Spectator.res", "GAME");
	InvalidateLayout();
}

CSpectatorGUI::~CSpectatorGUI(void)
{
	g_pSpectatorGUI = NULL;
}

void CSpectatorGUI::ApplySchemeSettings(IScheme *pScheme)
{
	m_pBottomBarBlank->SetVisible(true);
	m_pTopBar->SetVisible(true);

	BaseClass::ApplySchemeSettings(pScheme);

	m_pTopBar->SetBgColor(GetBlackBarColor());
	m_pBottomBarBlank->SetBgColor(GetBlackBarColor());

	SetBgColor(Color(0, 0, 0, 0));
	SetPaintBorderEnabled(false);

	SetBorder(NULL);
}

void CSpectatorGUI::PerformLayout(void)
{
	int w, h, x, y;
	surface()->GetScreenSize(w, h);
	SetBounds(0, 0, w, h);

	m_pBottomBarBlank->GetPos(x, y);
	m_pBottomBarBlank->SetSize(w, h - y);
}

void CSpectatorGUI::OnThink(void)
{
	BaseClass::OnThink();
}

void CSpectatorGUI::SetLogoImage(const char *image)
{
	if (m_pBannerImage)
		m_pBannerImage->SetImage(scheme()->GetImage(image, false));
}

void CSpectatorGUI::SetLabelText(const char *textEntryName, const char *text)
{
	Label *entry = dynamic_cast<Label *>(FindChildByName(textEntryName));

	if (entry)
		entry->SetText(text);
}

void CSpectatorGUI::SetLabelText(const char *textEntryName, wchar_t *text)
{
	Label *entry = dynamic_cast<Label *>(FindChildByName(textEntryName));

	if (entry)
		entry->SetText(text);
}

void CSpectatorGUI::MoveLabelToFront(const char *textEntryName)
{
	Label *entry = dynamic_cast<Label *>(FindChildByName(textEntryName));

	if (entry)
		entry->MoveToFront();
}

void CSpectatorGUI::ShowPanel(bool bShow)
{
	if (bShow && !IsVisible())
	{
		m_bSpecScoreboard = false;
	}

	SetVisible(bShow);

	if (!bShow && m_bSpecScoreboard)
	{
		if (g_pViewPort->IsScoreBoardVisible())
			g_pViewPort->ShowPanel(PANEL_SCOREBOARD, false);
	}
}

bool CSpectatorGUI::ShouldShowPlayerLabel(int specmode)
{
	if (g_pViewPort->IsSpectatorBarVisible())
		return false;

	return ((specmode == OBS_CHASE_LOCKED) || (specmode == OBS_CHASE_FREE) || (specmode == OBS_IN_EYE) || (specmode == OBS_MAP_CHASE));
}

void CSpectatorGUI::Paint(void)
{
	BaseClass::Paint();

	if (gHUD.m_Spectator.m_pip->value)
	{
		int mx, my, mwide, mtall;

		mx = gHUD.m_Spectator.m_OverviewData.insetWindowX;
		my = gHUD.m_Spectator.m_OverviewData.insetWindowY;
		mwide = gHUD.m_Spectator.m_OverviewData.insetWindowWidth;
		mtall = gHUD.m_Spectator.m_OverviewData.insetWindowHeight;

		surface()->DrawSetColor(255, 255, 255, 255);
		surface()->DrawFilledRect(mx - 1, my -1, mx + mwide, my);
		surface()->DrawFilledRect(mx + mwide, my - 1, mx + mwide + 1, my + mtall);
		surface()->DrawFilledRect(mx, my + mtall - 1, mx + mwide, my + mtall);
		surface()->DrawFilledRect(mx - 1, my, mx, my + mtall);
	}
}

bool CSpectatorGUI::NeedsUpdate(void)
{
	if (m_bLastSpecPic != (bool)gHUD.m_Spectator.m_pip->value)
		return true;

	return false;
}

void CSpectatorGUI::Update(void)
{
	if (!IsVisible())
		return;

	if (!m_bHelpShown && m_pBottomBarBlank->IsVisible() && g_iUser1 != 0)
	{
		m_bHelpShown = true;
		DuckMessage("#Spec_Duck");
	}

	gHUD.m_Spectator.CheckSettings();

	int wide, tall;
	surface()->GetScreenSize(wide, tall);

	int bx, by, bwide, btall;
	m_pTopBar->GetBounds(bx, by, bwide, btall);

	int specmode = g_iUser1;
	int playernum = g_iUser2;

	if (gHUD.m_Spectator.m_pip->value)
	{
		int mx, my, mwide, mtall;

		mx = gHUD.m_Spectator.m_OverviewData.insetWindowX;
		my = gHUD.m_Spectator.m_OverviewData.insetWindowY;
		mwide = gHUD.m_Spectator.m_OverviewData.insetWindowWidth;
		mtall = gHUD.m_Spectator.m_OverviewData.insetWindowHeight;

		if (my < btall)
		{
			m_pTopBar->SetSize(wide - (mx + mwide) - 2, btall);
			m_pTopBar->SetPos((mx + mwide) + 2, 0);
		}
		else
		{
			m_pTopBar->SetSize(wide, btall);
			m_pTopBar->SetPos(0, 0);
		}
	}
	else
	{
		m_pTopBar->SetSize(wide, btall);
		m_pTopBar->SetPos(0, 0);
	}

	m_bLastSpecPic = gHUD.m_Spectator.m_pip->value;
	m_pPlayerLabel->SetVisible(ShouldShowPlayerLabel(specmode));

	if (playernum > 0 && playernum <= gEngfuncs.GetMaxClients())
	{
		Color c = g_pViewPort->GetTeamColor(g_PlayerExtraInfo[playernum].teamnumber);

		m_pPlayerLabel->SetFgColor(c);

		wchar_t playerText[80], playerName[64], health[10];
		wcscpy(playerText, L"Unable to find #Spec_PlayerItem*");
		memset(playerName, 0x0, sizeof(playerName));

		const char *oldName = g_PlayerInfoList[playernum].name;
		Assert(oldName != NULL);
		int bufsize = strlen(oldName) * 2 + 1;
		char *newName = (char *)_alloca(bufsize);
		gViewPortInterface->MakeSafeName(oldName, newName, bufsize);
		g_pVGuiLocalize->ConvertANSIToUnicode(newName, playerName, sizeof(playerName));

		int iHealth = g_PlayerExtraInfo[playernum].health;

		if (iHealth > 0)
		{
			_snwprintf(health, sizeof(health), L"%i", iHealth);
			g_pVGuiLocalize->ConstructString(playerText, sizeof(playerText), g_pVGuiLocalize->Find("#Spec_PlayerItem_Team"), 2, playerName, health);
		}
		else
		{
			g_pVGuiLocalize->ConstructString(playerText, sizeof(playerText), g_pVGuiLocalize->Find("#Spec_PlayerItem"), 1, playerName);
		}

		m_pPlayerLabel->SetText(playerText);
	}
	else
	{
		m_pPlayerLabel->SetText(L"");
	}

	char tempstr[128];
	Q_FileBase(gEngfuncs.pfnGetLevelName(), tempstr, sizeof(tempstr));

	wchar_t wMapName[64];
	g_pVGuiLocalize->ConvertANSIToUnicode(tempstr, wMapName, sizeof(wMapName));

	wchar_t szEtxraInfo[1024], szTitleLabel[1024];
	g_pVGuiLocalize->ConstructString(szEtxraInfo, sizeof(szEtxraInfo), g_pVGuiLocalize->Find("#Spec_Map"), 1, wMapName);
	g_pVGuiLocalize->ConvertANSIToUnicode("", szTitleLabel, sizeof(szTitleLabel));

	SetLabelText("extrainfo", szEtxraInfo);
	SetLabelText("titlelabel", szTitleLabel);
}

void CSpectatorGUI::UpdateTimer(void)
{
	int timer = (int)(gHUD.m_roundTimer.m_flTimeEnd - gHUD.m_flTime);

	if (timer < 0)
		timer = 0;

	wchar_t szText[63];
	_snwprintf(szText, sizeof(szText), L"%d:%02d", (timer / 60), (timer % 60));
	szText[62] = 0;

	SetLabelText("timerlabel", szText);
}