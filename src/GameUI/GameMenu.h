#ifndef GAMEMENU_H
#define GAMEMENU_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Menu.h>
#include <vgui_controls/KeyRepeat.h>

class CGameMenu : public vgui::Menu
{
	DECLARE_CLASS_SIMPLE(CGameMenu, vgui::Menu);

public:
	CGameMenu(vgui::Panel *parent, const char *name);

public:
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void LayoutMenuBorder(void);

	void SetVisible(bool state);
	int AddMenuItem(const char *itemName, const char *itemText, const char *command, Panel *target, KeyValues *userData = NULL);
	void SetMenuItemBlinkingState(const char *itemName, bool state);
	void UpdateMenuItemState(bool isInGame, bool isMultiplayer);

	void OnCommand(const char *command);
	void OnKeyCodePressed(vgui::KeyCode code);
	void OnKeyCodeReleased(vgui::KeyCode code);
	void OnThink(void);
	void OnKillFocus(void);

private:
	vgui::CKeyRepeatHandler m_KeyRepeat;
};

#endif