#include "EngineInterface.h"
#include "GameMenu.h"

#include <KeyValues.h>
#include <vgui/IImage.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Menu.h>
#include <vgui_controls/MenuItem.h>

#include <IGameUIFuncs.h>
#include <ICommandLine.h>

#include "keydefs.h"

using namespace vgui;

class CGameMenuItem : public vgui::MenuItem
{
	DECLARE_CLASS_SIMPLE(CGameMenuItem, vgui::MenuItem);

public:
	CGameMenuItem(vgui::Menu *parent, const char *name) : BaseClass(parent, name, "GameMenuItem")
	{
		m_bRightAligned = false;
	}

public:
	void ApplySchemeSettings(vgui::IScheme *pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);

		SetFgColor(GetSchemeColor("MainMenu.TextColor", pScheme));
		SetBgColor(Color(0, 0, 0, 0));
		SetDefaultColor(GetSchemeColor("MainMenu.TextColor", pScheme), Color(0, 0, 0, 0));
		SetArmedColor(GetSchemeColor("MainMenu.ArmedTextColor", pScheme), Color(0, 0, 0, 0));
		SetDepressedColor(GetSchemeColor("MainMenu.DepressedTextColor", pScheme), Color(0, 0, 0, 0));
		SetContentAlignment(Label::a_west);
		SetBorder(NULL);
		SetDefaultBorder(NULL);
		SetDepressedBorder(NULL);
		SetKeyFocusBorder(NULL);
		SetFont(pScheme->GetFont("MenuLarge", IsProportional()));

		SetTextInset(0, 0);
		SetArmedSound("UI/buttonrollover.wav");
		SetDepressedSound("UI/buttonclick.wav");
		SetReleasedSound("UI/buttonclickrelease.wav");
		SetButtonActivationType(Button::ACTIVATE_ONPRESSED);

		if (m_bRightAligned)
		{
			SetContentAlignment(Label::a_east);
		}
	}

	void SetRightAlignedText(bool state)
	{
		m_bRightAligned = state;
	}

private:
	bool m_bRightAligned;
};

CGameMenu::CGameMenu(vgui::Panel *parent, const char *name) : BaseClass(parent, name)
{
}

void CGameMenu::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetMenuItemHeight(atoi(pScheme->GetResourceString("MainMenu.MenuItemHeight")));
	SetBgColor(Color(0, 0, 0, 0));
	SetBorder(NULL);
}

void CGameMenu::LayoutMenuBorder(void)
{
}

void CGameMenu::SetVisible(bool state)
{
	if (!IsVisible())
		BaseClass::SetVisible(true);

	if (!state)
	{
		vgui::ipanel()->MoveToBack(GetVPanel());

		if (!GetParent()->IsVisible())
			SetCurrentlySelectedItem(-1);
	}
}

int CGameMenu::AddMenuItem(const char *itemName, const char *itemText, const char *command, Panel *target, KeyValues *userData)
{
	MenuItem *item = new CGameMenuItem(this, itemName);
	item->AddActionSignalTarget(target);
	item->SetCommand(command);
	item->SetText(itemText);
	item->SetUserData(userData);

	return BaseClass::AddMenuItem(item);
}

void CGameMenu::SetMenuItemBlinkingState(const char *itemName, bool state)
{
	for (int i = 0; i < GetChildCount(); i++)
	{
		Panel *child = GetChild(i);
		MenuItem *menuItem = dynamic_cast<MenuItem *>(child);

		if (menuItem)
		{
			if (Q_strcmp(menuItem->GetCommand()->GetString("command", ""), itemName) == 0)
				menuItem->SetBlink(state);
		}
	}

	InvalidateLayout();
}

void CGameMenu::OnCommand(const char *command)
{
	m_KeyRepeat.Reset();

	if (!stricmp(command, "Open"))
	{
		MoveToFront();
		RequestFocus();
	}
	else
	{
		BaseClass::OnCommand(command);
	}
}

void CGameMenu::OnKeyCodePressed(vgui::KeyCode code)
{
	m_KeyRepeat.KeyDown(code);

	if (IsPC() && code >= KEY_F1 && code <= KEY_F12)
	{
		const char *binding = gameuifuncs->Key_BindingForKey(K_F1 + (code - KEY_F1));

		if (binding && binding[0])
		{
			char szCommand[256];
			Q_strncpy(szCommand, binding, sizeof(szCommand));
			engine->pfnClientCmd(szCommand);
		}
	}

	BaseClass::OnKeyCodePressed(code);
}

void CGameMenu::OnKeyCodeReleased(vgui::KeyCode code)
{
	m_KeyRepeat.KeyUp(code);

	BaseClass::OnKeyCodeReleased(code);
}

void CGameMenu::OnThink(void)
{
	vgui::KeyCode code = m_KeyRepeat.KeyRepeated();

	if (code)
		OnKeyCodeTyped(code);

	BaseClass::OnThink();
}

void CGameMenu::OnKillFocus(void)
{
	BaseClass::OnKillFocus();

	surface()->MovePopupToBack(GetVPanel());

	m_KeyRepeat.Reset();
}

void CGameMenu::UpdateMenuItemState(bool isInGame, bool isMultiplayer)
{
	bool isSteam = IsPC() && (CommandLine()->CheckParm("-steam") != 0);
	bool needUpdate = false;

	for (int i = 0; i < GetChildCount(); i++)
	{
		Panel *child = GetChild(i);
		MenuItem *menuItem = dynamic_cast<MenuItem *>(child);

		if (menuItem)
		{
			bool shouldBeVisible = true;
			KeyValues *kv = menuItem->GetUserData();

			if (!kv)
				continue;

			if (!isInGame && kv->GetInt("OnlyInGame"))
				shouldBeVisible = false;
			else if (isMultiplayer && kv->GetInt("notmulti"))
				shouldBeVisible = false;
			else if (isInGame && !isMultiplayer && kv->GetInt("notsingle"))
				shouldBeVisible = false;
			else if (isSteam && kv->GetInt("notsteam"))
				shouldBeVisible = false;
			else if (kv->GetInt("ConsoleOnly"))
				shouldBeVisible = false;

			if (menuItem->IsVisible() != shouldBeVisible)
			{
				menuItem->SetVisible(shouldBeVisible);
				needUpdate = true;
			}
		}
	}

	if (!isInGame)
	{
	}
	else
	{
		for (int i = 0; i < GetChildCount(); i++)
		{
			for (int j = i; j < GetChildCount() - 2; j++)
			{
				int iID1 = GetMenuID(j);
				int iID2 = GetMenuID(j + 1);

				MenuItem *menuItem1 = GetMenuItem(iID1);
				MenuItem *menuItem2 = GetMenuItem(iID2);

				KeyValues *kv1 = menuItem1->GetUserData();
				KeyValues *kv2 = menuItem2->GetUserData();

				if (kv1->GetInt("InGameOrder") > kv2->GetInt("InGameOrder"))
				{
					MoveMenuItem(iID2, iID1);
					needUpdate = true;
				}
			}
		}
	}

	if (needUpdate)
	{
		InvalidateLayout();
	}
}