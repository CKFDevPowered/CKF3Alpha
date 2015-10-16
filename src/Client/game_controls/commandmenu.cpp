#include "hud.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "vgui_int.h"

#include "CounterStrikeViewport.h"

#include "CommandMenu.h"
#include "vgui_controls/MenuItem.h"

CommandMenu::CommandMenu(Panel *parent, const char *panelName) : Menu(parent, panelName)
{
	SetVisible(false);

	m_CurrentMenu = this;
	m_MenuKeys = NULL;
	m_nCloseKey = KEY_NONE;
}

void CommandMenu::VidInit(void)
{
	RebuildMenu();
}

bool CommandMenu::LoadFromFile(const char *fileName)
{
	KeyValues *kv = new KeyValues(fileName);

	if (!kv->LoadFromFile(g_pFullFileSystem, fileName, "GAME"))
		return false;

	bool ret = LoadFromKeyValues(kv);

	kv->deleteThis();
	return ret;
}

CommandMenu::~CommandMenu(void)
{
	ClearMenu();
}

void CommandMenu::OnKeyCodeTyped(vgui::KeyCode code)
{
	BaseClass::OnKeyCodeTyped(code);

	if (code == m_nCloseKey)
	{
		SetVisible(false);
	}
}

void CommandMenu::OnMessage(const KeyValues *params, VPANEL fromPanel)
{
	char text[255];
	bool bHandled = false;

	KeyValues *param1 = const_cast<KeyValues *>(params);

	Q_strncpy(text, param1->GetString("toggle"), sizeof(text));

	if (text[0])
	{
		if (gEngfuncs.pfnGetCvarFloat(text))
			gEngfuncs.Cvar_SetValue(text, 0);
		else
			gEngfuncs.Cvar_SetValue(text, 1);

		UpdateMenu();
		bHandled = true;
	}

	Q_strncpy(text, param1->GetString("command"), sizeof(text));

	if (text[0])
	{
		if (text[0] == '!')
		{
			if (!strcmp(&text[1], "MAPBRIEFING"))
			{
				g_pViewPort->ShowVGUIMenu(MENU_MAPBRIEFING);
				return;
			}
		}
		else
		{
			gEngfuncs.pfnClientCmd(text);
		}

		bHandled = true;
	}

	Q_strncpy(text, param1->GetString("custom"), sizeof(text));

	if (text[0])
	{
		OnCustomItem(param1);
		bHandled = true;
	}

	if (bHandled)
		PostMessage(GetParent(), new KeyValues("CommandMenuClosed"));

	BaseClass::OnMessage(params, fromPanel);
}

void CommandMenu::StartNewSubMenu(KeyValues *params)
{
	CommandMenuItem menuitem;
	menuitem.menu = m_CurrentMenu;

	Menu *menu = new Menu(this, params->GetString("name"));

	menuitem.itemnr = m_CurrentMenu->AddCascadingMenuItem(params->GetString("label"), this, menu, params);

	m_MenuItems.AddToTail(menuitem);
	m_pMenuStack.Push(m_CurrentMenu);
	m_CurrentMenu = menu;
}

void CommandMenu::FinishSubMenu(void)
{
	m_CurrentMenu = m_pMenuStack.Top();
	m_pMenuStack.Pop();
}

void CommandMenu::AddMenuCommandItem(KeyValues *params)
{
	CommandMenuItem menuitem;
	menuitem.menu = m_CurrentMenu;
	menuitem.itemnr = m_CurrentMenu->AddMenuItem(params->GetString("label"), params->MakeCopy(), this, params);
	m_MenuItems.AddToTail(menuitem);
}

void CommandMenu::AddMenuToggleItem(KeyValues *params)
{
	CommandMenuItem menuitem;
	menuitem.menu = m_CurrentMenu;
	menuitem.itemnr = m_CurrentMenu->AddCheckableMenuItem(params->GetString("label"), params->MakeCopy(), this, params);
	m_MenuItems.AddToTail(menuitem);
}

void CommandMenu::AddMenuCustomItem(KeyValues *params)
{
	CommandMenuItem menuitem;
	menuitem.menu = m_CurrentMenu;
	menuitem.itemnr = AddCustomItem(params, m_CurrentMenu);
	m_MenuItems.AddToTail(menuitem);
}

void CommandMenu::ClearMenu(void)
{
	SetVisible(false);
	m_pMenuStack.Clear();
	m_MenuItems.RemoveAll();

	MarkForDeletion();

	if (m_MenuKeys)
	{
		m_MenuKeys->deleteThis();
		m_MenuKeys = NULL;
	}
}

void CommandMenu::RebuildMenu(void)
{
	if (!m_MenuKeys)
		return;

	m_pMenuStack.Clear();
	m_MenuItems.RemoveAll();
	DeleteAllItems();

	LoadFromKeyValues(m_MenuKeys);
}

void CommandMenu::UpdateMenu(void)
{
	if (Q_strlen(m_CurrentMap) == 0)
	{
		RebuildMenu();
		return;
	}

	char text[255];
	int num = m_MenuItems.Count();

	for (int i = 0; i < num; i++)
	{
		CommandMenuItem menuitem = m_MenuItems.Element(i);
		KeyValues *keys = menuitem.menu->GetItemUserData(menuitem.itemnr);

		if (!keys)
			continue;

		Q_strncpy(text, keys->GetString("custom"), sizeof(text));

		if (text[0])
		{
			UpdateCustomItem(keys, menuitem.menu->GetMenuItem(menuitem.itemnr));
			continue;
		}

		Q_strncpy(text, keys->GetString("toggle"), sizeof(text));

		if (text[0])
			menuitem.menu->SetMenuItemChecked(menuitem.itemnr, gEngfuncs.pfnGetCvarFloat(text));
	}

	InvalidateLayout(true);
}

void CommandMenu::SetVisible(bool state)
{
	if (state == IsVisible())
		return;

	BaseClass::SetVisible(state);

	if (state)
		UpdateMenu();
}

bool CommandMenu::CheckRules(const char *rule, const char *ruledata)
{
	if (!rule || !ruledata)
		return true;

	if (Q_strlen(m_CurrentMap) == 0)
		return false;

	if (Q_strcmp(rule, "team") == 0)
	{
		return (Q_strcmp(m_CurrentTeam, ruledata) == 0);
	}
	else if (Q_strcmp(rule, "map") == 0)
	{
		return (Q_strcmp(m_CurrentMap, ruledata) == 0);
	}
	else if (Q_strcmp(rule, "objective") == 0)
	{
		if (Q_strcmp(ruledata, "hostage") == 0 && Q_strncmp(m_CurrentMap, "cs_", 3) == 0)
			return true;

		if (Q_strcmp(ruledata, "bomb") == 0 && Q_strncmp(m_CurrentMap, "de_", 3) == 0)
			return true;

		if (Q_strcmp(ruledata, "vip") == 0 && Q_strncmp(m_CurrentMap, "as_", 3) == 0)
			return true;

		if (Q_strcmp(ruledata, "escape") == 0 && Q_strncmp(m_CurrentMap, "aes_", 3) == 0)
			return true;

		return false;
	}

	return true;
}

KeyValues *CommandMenu::GetKeyValues(void)
{
	return m_MenuKeys;
}

bool CommandMenu::LoadFromKeyValues(KeyValues *params)
{
	if (!params)
		return false;

	Q_snprintf(m_CurrentTeam, 4, "%i", *gCKFVars.g_iTeam );
	Q_FileBase(gEngfuncs.pfnGetLevelName(), m_CurrentMap, sizeof(m_CurrentMap));

	if (params != m_MenuKeys)
	{
		if (m_MenuKeys)
			m_MenuKeys->deleteThis();

		m_MenuKeys = params->MakeCopy();
	}

	KeyValues *subkey = m_MenuKeys->GetFirstSubKey();

	while (subkey)
	{
		if (subkey->GetDataType() == KeyValues::TYPE_NONE)
		{
			if (!LoadFromKeyValuesInternal(subkey, 0))
				return false;
		}

		subkey = subkey->GetNextKey();
	}

	if (Q_strlen(m_CurrentMap) == 0)
		return false;

	UpdateMenu();
	return true;
}

bool CommandMenu::LoadFromKeyValuesInternal(KeyValues *key, int depth)
{
	char text[255];
	KeyValues *subkey = NULL;

	if (depth > 100)
	{
		Msg("CommandMenu::LoadFromKeyValueInternal: depth > 100.\n");
		return false;
	}

	Q_strncpy(text, key->GetString("custom"), sizeof(text));

	if (text[0])
	{
		AddMenuCustomItem(key);
		return true;
	}

	if (!CheckRules(key->GetString("rule"), key->GetString("ruledata")))
		return true;

	Q_strncpy(text, key->GetString("toggle"), sizeof(text));

	if (text[0])
	{
		AddMenuToggleItem(key);
		return true;
	}

	Q_strncpy(text, key->GetString("command"), sizeof(text));

	if (text[0])
	{
		AddMenuCommandItem(key);
		return true;
	}

	StartNewSubMenu(key);

	subkey = key->GetFirstSubKey();

	while (subkey)
	{
		if (subkey->GetDataType() == KeyValues::TYPE_NONE)
		{
			LoadFromKeyValuesInternal(subkey, depth + 1);
		}

		subkey = subkey->GetNextKey();
	}

	FinishSubMenu();
	return true;
}