#include "hud.h"
#include "csbtegraphicmenu.h"
#include "cdll_dll.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <FileSystem.h>

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/Panel.h>

#include "mouseoverpanelbutton_new.h"

using namespace vgui;

CCSBTEGraphicMenu::CCSBTEGraphicMenu(void) : CClassMenu(PANEL_GRAPHICMENU)
{
	LoadControlSettingsFromScheme("UI/GraphicMenu.res");
}

const char *CCSBTEGraphicMenu::GetName(void)
{
	return PANEL_GRAPHICMENU;
}

void CCSBTEGraphicMenu::Reset(void)
{
	CClassMenu::Reset();
}

void CCSBTEGraphicMenu::ShowPanel(bool bShow)
{
	if (BaseClass::IsVisible() == bShow)
		return;

	if (bShow)
	{
		if (gHUD.m_iIntermission || gEngfuncs.IsSpectateOnly())
			return;
	}

	BaseClass::ShowPanel(bShow);

	if (bShow)
	{
		g_pViewPort->HideVoiceLabels();
		g_pViewPort->SetChatDialogVisible(false);
	}
}

void CCSBTEGraphicMenu::SetVisible(bool state)
{
	BaseClass::SetVisible(state);
}

void CCSBTEGraphicMenu::Update(void)
{
}

Panel *CCSBTEGraphicMenu::CreateControlByName(const char *controlName)
{
	return BaseClass::CreateControlByName(controlName);
}

void CCSBTEGraphicMenu::PaintBackground(void)
{
}

void CCSBTEGraphicMenu::PerformLayout(void)
{
	BaseClass::PerformLayout();
}

void CCSBTEGraphicMenu::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

void CCSBTEGraphicMenu::Setup(int flags)
{
	Label *CancelButton = (Label *)FindChildByName("CancelButton");

	if (CancelButton)
	{
		if (flags & MENU_FLAG_EXIT)
		{
			CancelButton->SetEnabled(true);
			CancelButton->SetVisible(true);
		}
		else if (flags & MENU_FLAG_EXIT_DISABLE)
		{
			CancelButton->SetEnabled(false);
			CancelButton->SetVisible(true);
		}
		else
		{
			CancelButton->SetEnabled(false);
		}
	}
}

void CCSBTEGraphicMenu::SetTitle(const char *title)
{
	Label *menuTitle = (Label *)FindChildByName("menuTitle");

	if (!menuTitle)
		return;

	menuTitle->SetText(title);
}

char *CCSBTEGraphicMenu::GetItemName(int slot)
{
	static char name[12];
	sprintf(name, "menuItem%d", slot);
	return name;
}

void CCSBTEGraphicMenu::ClearItems(void)
{
	for (int i = 1; i <= 9; i++)
	{
		MouseOverPanelButton *button = (MouseOverPanelButton *)FindChildByName(GetItemName(i));

		if (!button)
			continue;

		button->SetVisible(false);
	}
}

void CCSBTEGraphicMenu::AddItem(int slot, const char *text, bool enabled)
{
	MouseOverPanelButton *button = (MouseOverPanelButton *)FindChildByName(GetItemName(slot));

	if (!button)
		return;

	char textBuffer[256];
	sprintf(textBuffer, "&%d %s", slot, text);

	button->SetVisible(true);
	button->SetText(textBuffer);
	button->SetEnabled(enabled);
}