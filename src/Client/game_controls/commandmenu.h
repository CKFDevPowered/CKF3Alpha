#ifndef COMMANDMENU_H
#define COMMANDMENU_H

#include <vgui_controls/Menu.h>
#include <filesystem.h>
#include "UtlStack.h"
#include "UtlVector.h"
#include <KeyValues.h>

using namespace vgui;

#define PANEL_CMDMENU "cmdmenu"

class CommandMenu : public Menu, public CViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE(CommandMenu, Menu);

	typedef struct
	{
		Menu *menu;
		int itemnr;
	}
	CommandMenuItem;

public:
	CommandMenu(Panel *parent = NULL, const char *panelName = "CommandMenu");
	~CommandMenu(void);

public:
	bool LoadFromFile(const char *fileName);
	void UpdateMenu(void);
	void RebuildMenu(void);
	void ClearMenu(void);

public:
	virtual void Init(void) {}
	virtual void VidInit(void);
	virtual void Reset(void) {}
	virtual void Update(void) {}
	virtual const char *GetName(void) { return PANEL_CMDMENU; }
	virtual void SetData(KeyValues *data) {}
	virtual bool NeedsUpdate(void) { return false; }
	virtual bool HasInputElements(void) { return true; }
	virtual void ShowPanel(bool bShow) { SetVisible(bShow); }

public:
	DECLARE_VIEWPORT_PANEL_SIMPLE();

public:
	virtual int AddCustomItem(KeyValues *params, Menu *menu) { return 0; }
	virtual void UpdateCustomItem(KeyValues *params, MenuItem *item) {}
	virtual void OnCustomItem(KeyValues *params) {}
	virtual bool CheckRules(const char *rule, const char *ruledata);
	virtual void SetVisible(bool state);
	virtual void SetCloseKey(vgui::KeyCode code) { m_nCloseKey = code; }

protected:
	void OnKeyCodeTyped(vgui::KeyCode code);
	void OnMessage(const KeyValues *params, VPANEL fromPanel);

protected:
	void StartNewSubMenu(KeyValues * params);
	void FinishSubMenu(void);
	void AddMenuCommandItem(KeyValues *params);
	void AddMenuCustomItem(KeyValues *params);
	void AddMenuToggleItem(KeyValues *params);
	bool LoadFromKeyValuesInternal(KeyValues *key, int depth);
	bool LoadFromKeyValues(KeyValues *key);
	KeyValues *GetKeyValues(void);

protected:
	Menu *m_CurrentMenu;
	char m_CurrentTeam[4];
	char m_CurrentMap[256];
	KeyCode m_nCloseKey;

	KeyValues *m_MenuKeys;

	CUtlStack<vgui::Menu *> m_pMenuStack;
	CUtlVector<CommandMenuItem> m_MenuItems;
};

#endif