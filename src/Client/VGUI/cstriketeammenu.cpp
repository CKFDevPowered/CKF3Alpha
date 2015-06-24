#include "hud.h"
#include "cstriketeammenu.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <FileSystem.h>

#include <cl_entity.h>
#include <cdll_dll.h>

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Panel.h>

#include "CounterStrikeViewport.h"

using namespace vgui;

CCSTeamMenu::CCSTeamMenu(void) : CTeamMenu()
{
	LoadControlSettings("Resource/UI/TeamMenu.res");
	InvalidateLayout();
}

CCSTeamMenu::~CCSTeamMenu(void)
{
}

MouseOverPanelButton *CCSTeamMenu::CreateNewMouseOverPanelButton(EditablePanel *panel)
{
	return new MouseOverPanelButton(this, "MouseOverPanelButton", panel);
}

void CCSTeamMenu::ShowPanel(bool bShow)
{
	if (bShow)
	{
		if (gHUD.m_iIntermission || gEngfuncs.IsSpectateOnly())
			return;
	}

	BaseClass::ShowPanel(bShow);
}

void CCSTeamMenu::Update(void)
{
	BaseClass::Update();

	if (gViewPortInterface->GetAllowSpectators())
	{
		if (g_iTeamNumber == TEAM_UNASSIGNED || g_iFreezeTimeOver || (g_PlayerExtraInfo[gHUD.m_iPlayerNum].dead))
			SetVisibleButton("specbutton", true);
		else
			SetVisibleButton("specbutton", false);
	}
	else
		SetVisibleButton("specbutton", false);

	m_bVIPMap = false;

	char mapName[32];
	Q_FileBase(engine->pfnGetLevelName(), mapName, sizeof(mapName));

	if (!Q_strncmp(mapName, "as_", 3))
		m_bVIPMap = true;

	if (!m_bVIPMap || (g_iTeamNumber != TEAM_CT))
		SetVisibleButton("vipbutton", false);
	else
		SetVisibleButton("vipbutton", true);

	if (g_iTeamNumber == TEAM_UNASSIGNED)
		SetVisibleButton("CancelButton", false);
	else
		SetVisibleButton("CancelButton", true);
}

void CCSTeamMenu::SetVisible(bool state)
{
	BaseClass::SetVisible(state);

	if (state)
	{
		Panel *pAutoButton = FindChildByName("autobutton");

		if (pAutoButton)
			pAutoButton->RequestFocus();
	}
}

void CCSTeamMenu::OnCommand(const char *command)
{
	if (Q_stricmp(command, "vguicancel"))
	{
		engine->pfnClientCmd(const_cast<char *>(command));
	}

	Close();

	g_pViewPort->ShowBackGround(false);

	BaseClass::OnCommand(command);
}

void CCSTeamMenu::SetVisibleButton(const char *textEntryName, bool state)
{
	Button *entry = dynamic_cast<Button *>(FindChildByName(textEntryName));

	if (entry)
		entry->SetVisible(state);
}

void CCSTeamMenu::PaintBackground(void)
{
	BaseClass::PaintBackground();
}

void CCSTeamMenu::PerformLayout(void)
{
	BaseClass::PerformLayout();
}

void CCSTeamMenu::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}