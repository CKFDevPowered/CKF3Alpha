#include "hud.h"
#include "cstrikeclassmenu.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <FileSystem.h>

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Panel.h>

using namespace vgui;

CCSClassMenu_TER::CCSClassMenu_TER(void) : CClassMenu(PANEL_CLASS_TER)
{
	LoadControlSettings("Resource/UI/ClassMenu_TER.res");
}

const char *CCSClassMenu_TER::GetName(void)
{
	return PANEL_CLASS_TER;
}

void CCSClassMenu_TER::Reset(void)
{
	CClassMenu::Reset();
}

MouseOverPanelButton *CCSClassMenu_TER::CreateNewMouseOverPanelButton(vgui::EditablePanel *panel)
{
	return new MouseOverPanelButton(this, "MouseOverPanelButton", panel);
}

void CCSClassMenu_TER::ShowPanel(bool bShow)
{
	if (bShow)
	{
		if (gHUD.m_iIntermission || gEngfuncs.IsSpectateOnly())
			return;
	}

	BaseClass::ShowPanel(bShow);
}

void CCSClassMenu_TER::SetVisible(bool state)
{
	BaseClass::SetVisible(state);

	if (state)
	{
		Panel *pAutoButton = FindChildByName("autoselect_t");

		if (pAutoButton)
			pAutoButton->RequestFocus();
	}
}

void CCSClassMenu_TER::Update(void)
{
	SetVisibleButton("CancelButton", false);
}

CCSClassMenu_CT::CCSClassMenu_CT(void) : CClassMenu(PANEL_CLASS_CT)
{
	LoadControlSettings("Resource/UI/ClassMenu_CT.res");
}

const char *CCSClassMenu_CT::GetName(void)
{
	return PANEL_CLASS_CT;
}

void CCSClassMenu_CT::Reset(void)
{
	CClassMenu::Reset();
}

MouseOverPanelButton *CCSClassMenu_CT::CreateNewMouseOverPanelButton(vgui::EditablePanel *panel)
{
	return new MouseOverPanelButton(this, "MouseOverPanelButton", panel);
}

void CCSClassMenu_CT::ShowPanel(bool bShow)
{
	if (bShow)
	{
		if (gHUD.m_iIntermission || gEngfuncs.IsSpectateOnly())
			return;
	}

	BaseClass::ShowPanel(bShow);
}

void CCSClassMenu_CT::SetVisible(bool state)
{
	BaseClass::SetVisible(state);

	if (state)
	{
		Panel *pAutoButton = FindChildByName("autoselect_ct");

		if (pAutoButton)
			pAutoButton->RequestFocus();
	}
}

void CCSClassMenu_CT::Update(void)
{
	SetVisibleButton("CancelButton", false);
}

void CCSClassMenu_TER::PaintBackground(void)
{
	BaseClass::PaintBackground();
}

void CCSClassMenu_TER::PerformLayout(void)
{
	BaseClass::PerformLayout();
}

void CCSClassMenu_TER::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

void CCSClassMenu_CT::PaintBackground(void)
{
	BaseClass::PaintBackground();
}

void CCSClassMenu_CT::PerformLayout(void)
{
	BaseClass::PerformLayout();
}

void CCSClassMenu_CT::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}