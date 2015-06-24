#include "hud.h"
#include "cdll_dll.h"
#include "cl_util.h"
#include "cstrikebuymenu.h"
#include "cstrikebuysubmenu.h"
#include "buy_presets/buy_presets.h"
#include "shared_util.h"
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include "buypreset_weaponsetlabel.h"
#include "vgui_controls/RichText.h"
#include "perf_counter.h"
#include "buymouseoverpanelbutton.h"
#include "buypresetbutton.h"

using namespace vgui;

CCSBuyMenu_CT::CCSBuyMenu_CT(void) : CCSBaseBuyMenu("BuySubMenu_CT")
{
	m_pMainMenu->LoadControlSettings("UI/BuyMenu_CT.res", "GAME");
	m_pMainMenu->SetVisible(false);

	m_iTeam = TEAM_CT;
}

CCSBuyMenu_TER::CCSBuyMenu_TER(void) : CCSBaseBuyMenu("BuySubMenu_TER")
{
	m_pMainMenu->LoadControlSettings("UI/BuyMenu_TER.res", "GAME");
	m_pMainMenu->SetVisible(false);

	m_iTeam = TEAM_TERRORIST;
}

CCSBaseBuyMenu::CCSBaseBuyMenu(const char *subPanelName) : CBuyMenu()
{
	SetTitle("#Cstrike_Buy_Menu", true);
	SetProportional(true);

	LoadControlSettings("UI/BuyMenu.res", "GAME");

	if (m_pMainMenu)
		delete m_pMainMenu;

	m_pMainMenu = new CCSBuySubMenu(this, subPanelName);
}

void CCSBaseBuyMenu::SetVisible(bool state)
{
	BaseClass::SetVisible(state);

	if (state)
	{
		Panel *defaultButton = FindChildByName("CancelButton");

		if (defaultButton)
			defaultButton->RequestFocus();

		SetMouseInputEnabled(true);
		m_pMainMenu->SetMouseInputEnabled(true);
	}
}

void CCSBaseBuyMenu::Init(void)
{
}

void CCSBaseBuyMenu::VidInit(void)
{
	SetVisible(false);
}

void CCSBaseBuyMenu::ShowPanel(bool bShow)
{
	if (bShow)
	{
		if (gHUD.m_iIntermission || gEngfuncs.IsSpectateOnly())
			return;
	}

	BaseClass::ShowPanel(bShow);
}

void CCSBaseBuyMenu::Paint(void)
{
	BaseClass::Paint();
}

void CCSBaseBuyMenu::PaintBackground(void)
{
	BaseClass::PaintBackground();
}

void CCSBaseBuyMenu::PerformLayout(void)
{
	BaseClass::PerformLayout();
}

void CCSBaseBuyMenu::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

void CCSBaseBuyMenu::GotoMenu(int iMenu)
{
	if (!m_pMainMenu)
		return;

	const char *command = NULL;

	switch (iMenu)
	{
		case MENU_BUY_PISTOL:
		{
			if (m_iTeam == TEAM_CT)
				command = "UI/BuyPistols_CT.res";
			else
				command = "UI/BuyPistols_TER.res";

			break;
		}

		case MENU_BUY_SHOTGUN:
		{
			if (m_iTeam == TEAM_CT)
				command = "UI/BuyEquipment_CT.res";
			else
				command = "UI/BuyEquipment_TER.res";

			break;
		}

		case MENU_BUY_RIFLE:
		{
			if (m_iTeam == TEAM_CT)
				command = "UI/BuyRifles_CT.res";
			else
				command = "UI/BuyRifles_TER.res";

			break;
		}

		case MENU_BUY_SUBMACHINEGUN:
		{
			if (m_iTeam == TEAM_CT)
				command = "UI/BuySubMachineguns_CT.res";
			else
				command = "UI/BuySubMachineguns_TER.res";

			break;
		}

		case MENU_BUY_MACHINEGUN:
		{
			command = "UI/BuyMachineguns.res";
			break;
		}

		case MENU_BUY_ITEM:
		{
			if (m_iTeam == TEAM_CT)
				command = "UI/BuyEquipment_CT.res";
			else
				command = "UI/BuyEquipment_TER.res";

			break;
		}
	}

	ActivateNextSubPanel(m_pMainMenu);

	if (command)
	{
		m_pMainMenu->SetupNextSubPanel(command);
		m_pMainMenu->GotoNextSubPanel();
	}
}

void CCSBaseBuyMenu::ActivateMenu(int iMenu)
{
	GotoMenu(iMenu);

	g_pViewPort->ShowPanel(this, true);
}

CCSBuySubMenu::CCSBuySubMenu(vgui::Panel *parent, const char *name) : CBuySubMenu(parent, name)
{
#if USE_BUY_PRESETS
	if (!TheBuyPresets)
		TheBuyPresets = new BuyPresetManager();

	for (int i = 0; i < NUM_BUY_PRESET_BUTTONS; ++i)
	{
		m_pBuyPresetButtons[i] = new BuyPresetButton(this, VarArgs("BuyPresetButton%c", 'A' + i));
		m_pBuyPresetButtons[i]->SetVisible(false);
	}

	m_pMoney = new Label(this, "money", "");
	m_pLoadout = new BuyPresetEditPanel(this, "loadoutPanel", "UI/Loadout.res", 0, false);
	m_pMoney->SetVisible(false);
	m_pLoadout->SetVisible(false);
#else
	for (int i = 0; i < NUM_BUY_PRESET_BUTTONS; ++i)
		m_pBuyPresetButtons[i] = NULL;

	m_pMoney = NULL;
	m_pMainBackground = NULL;
#endif
	m_lastMoney = -1;
}

void CCSBuySubMenu::OnCommand(const char *command)
{
#if USE_BUY_PRESETS
	const char *buyPresetSetString = "cl_buy_favorite_query_set ";

	if (!strnicmp(command, buyPresetSetString, strlen(buyPresetSetString)))
	{
		int index = atoi(command + strlen(buyPresetSetString)) - 1;

		TheBuyPresets->SetPurchasePreset(index);
		UpdateBuyPreset(index);
		return;
	}
#endif
	if (!Q_strcmp(command, "buy_unavailable"))
	{
		BaseClass::OnCommand("vguicancel");
		return;
	}

	BaseClass::OnCommand(command);
}

void CCSBuySubMenu::OnSizeChanged(int newWide, int newTall)
{
	BaseClass::OnSizeChanged(newWide, newTall);
}

void CCSBuySubMenu::OnDisplay(void)
{
	BaseClass::OnDisplay();
}

void CCSBuySubMenu::PerformLayout(void)
{
	BaseClass::PerformLayout();

#if USE_BUY_PRESETS
	if (IsVisible())
	{
		UpdateBuyPresets();
		UpdateLoadout();
	}
#endif
}

MouseOverPanelButton *CCSBuySubMenu::CreateNewMouseOverPanelButton(EditablePanel *panel)
{
	return new BuyMouseOverPanelButton(this, NULL, panel);
}

CBuySubMenu *CCSBuySubMenu::CreateNewSubMenu(void)
{
	return new CCSBuySubMenu(this, "BuySubMenu");
}

void CCSBuySubMenu::SetupNextSubPanel(const char *fileName)
{
	int i;

	for (i = 0; i < m_SubMenus.Count(); i++)
	{
		if (!Q_stricmp(m_SubMenus[i].filename, fileName))
		{
			m_NextPanel = m_SubMenus[i].panel;
			Assert(m_NextPanel);
			m_NextPanel->InvalidateLayout();
			return;
		}
	}

	if (i == m_SubMenus.Count())
	{
		SubMenuEntry_t newEntry;
		newEntry.filename[0] = 0;
		newEntry.panel = NULL;

		CBuySubMenu *newMenu = CreateNewSubMenu();
		newMenu->LoadControlSettings(fileName);
		m_NextPanel = newMenu;
		Q_strncpy(newEntry.filename, fileName, sizeof(newEntry.filename));
		newEntry.panel = newMenu;
		m_SubMenus.AddToTail(newEntry);
	}
}

void CCSBuySubMenu::UpdateBuyPreset(int index)
{
	if (!m_pBuyPresetButtons[index])
		return;

	const BuyPreset *preset = TheBuyPresets->GetPreset(index);

	if (!preset)
		return;

	int setIndex;
	int currentCost = -1;
	WeaponSet currentSet;
	const WeaponSet *fullSet = NULL;

	for (setIndex = 0; setIndex < preset->GetNumSets(); ++setIndex)
	{
		const WeaponSet *itemSet = preset->GetSet(setIndex);

		if (itemSet)
		{
			itemSet->GetCurrent(currentCost, currentSet);

			if (currentCost >= 0)
			{
				fullSet = itemSet;
				break;
			}
		}
	}

	m_pBuyPresetButtons[index]->ClearWeapons();

	if (!fullSet && preset->GetNumSets())
		fullSet = preset->GetSet(0);

	if (fullSet)
	{
		if (fullSet->GetPrimaryWeapon().GetWeaponID() != WEAPON_NONE)
			m_pBuyPresetButtons[index]->SetPrimaryWeapon(scheme()->GetImage(ImageFnameFromWeaponID(fullSet->GetPrimaryWeapon().GetWeaponID(), true), true));

		if (fullSet->GetSecondaryWeapon().GetWeaponID() != WEAPON_NONE)
			m_pBuyPresetButtons[index]->SetSecondaryWeapon(scheme()->GetImage(ImageFnameFromWeaponID(fullSet->GetSecondaryWeapon().GetWeaponID(), false), true));

		if (fullSet->GetKnifeWeapon().GetWeaponID() != WEAPON_NONE)
			m_pBuyPresetButtons[index]->SetKnifeWeapon(scheme()->GetImage(ImageFnameFromWeaponID(fullSet->GetKnifeWeapon().GetWeaponID(), false), true));
	}
}

void CCSBuySubMenu::UpdateBuyPresets(void)
{
	int numPresets = min(TheBuyPresets->GetNumPresets(), NUM_BUY_PRESET_BUTTONS);

	for (int i = 0; i < numPresets; ++i)
		UpdateBuyPreset(i);

	for (int i = numPresets + 1; i < NUM_BUY_PRESET_BUTTONS; ++i)
	{
		if (m_pBuyPresetButtons[i])
			m_pBuyPresetButtons[i]->SetVisible(false);
	}
}

void CCSBuySubMenu::UpdateLoadout(void)
{
	if (m_pLoadout)
	{
		WeaponSet ws;
		TheBuyPresets->GetCurrentLoadout(&ws);
		m_pLoadout->SetWeaponSet(&ws, true);
	}
}