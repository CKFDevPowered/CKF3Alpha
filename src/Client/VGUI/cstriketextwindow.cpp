#include "hud.h"
#include "cstriketextwindow.h"
#include "cstrikeclientscoreboard.h"
#include "parsemsg.h"

#include <vgui/IVGui.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <KeyValues.h>

#include <IGameUIFuncs.h>

#include <vgui_controls/Label.h>
#include <vgui_controls/BuildGroup.h>
#include <vgui_controls/BitmapImagePanel.h>
#include <vgui_controls/TextEntry.h>

#include "CounterStrikeViewport.h"

using namespace vgui;

CCSTextWindow::CCSTextWindow(void) : CTextWindow()
{
	SetProportional(true);

	m_iScoreBoardKey = KEY_NONE;
}

CCSTextWindow::~CCSTextWindow(void)
{
}

void CCSTextWindow::Init(void)
{
	m_szMOTD[0] = 0;
}

void CCSTextWindow::VidInit(void)
{
	BaseClass::VidInit();

	m_szMOTD[0] = 0;
}

void CCSTextWindow::Update(void)
{
	BaseClass::Update();

	m_pOK->RequestFocus();
}

void CCSTextWindow::SetVisible(bool state)
{
	BaseClass::SetVisible(state);

	if (state)
		m_pOK->RequestFocus();
}

void CCSTextWindow::ShowPanel(bool bShow)
{
	if (bShow)
	{
		if (gHUD.m_iIntermission || gEngfuncs.IsSpectateOnly())
			return;

		if (m_iScoreBoardKey == KEY_NONE)
			m_iScoreBoardKey = gameuifuncs->GetVGUI2KeyCodeForBind("showscores");
	}

	BaseClass::ShowPanel(bShow);
}

void CCSTextWindow::OnKeyCodePressed(KeyCode code)
{
	if (code == KEY_SPACE || code == KEY_ENTER)
	{
		OnCommand("okay");
	}
	else if (m_iScoreBoardKey != KEY_NONE && m_iScoreBoardKey == code)
	{
		g_pViewPort->ShowPanel(PANEL_SCOREBOARD, true);
		g_pViewPort->PostMessageToPanel(PANEL_SCOREBOARD, new KeyValues("PollHideCode", "code", code));
	}
	else
		BaseClass::OnKeyCodePressed(code);
}

void CCSTextWindow::PaintBackground(void)
{
	BaseClass::PaintBackground();
}

void CCSTextWindow::PerformLayout()
{
	BaseClass::PerformLayout();
}

void CCSTextWindow::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	LoadControlSettings("Resource/UI/TextWindow.res");
	Update();
}

void CCSTextWindow::ShowMapBriefing(void)
{
	char newname[32];
	const char *mapname = gEngfuncs.pfnGetLevelName();
	Q_FileBase(mapname, newname, sizeof(newname));
	Q_snprintf(m_szMOTD, sizeof(m_szMOTD), "maps/%s.txt", newname);

	if (!g_pFullFileSystem->FileExists(m_szMOTD))
	{
		if (g_pFullFileSystem->FileExists("maps/default.txt"))
			Q_snprintf(m_szMOTD, sizeof(m_szMOTD), "maps/default.txt");
	}

	SetData(TYPE_FILE, newname, m_szMOTD, NULL);
	ShowPanel(true);
}

void CCSTextWindow::ShowSpectateHelp(void)
{
	SetData(TYPE_TEXT, "#Spec_Help_Title", "#Spec_Help_Text", NULL);
	ShowPanel(true);
}

void CCSTextWindow::ShowMOTD(char *title, char *string)
{
	strcpy(m_szMOTD, string);

	SetData(TYPE_INDEX, g_pViewPort->GetServerName(), m_szMOTD, NULL);
	ShowPanel(true);
}