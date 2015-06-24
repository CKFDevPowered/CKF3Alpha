#ifndef CSBTEGRAPHICMENU_H
#define CSBTEGRAPHICMENU_H

#ifdef _WIN32
#pragma once
#endif

#include "game_controls\classmenu.h"
#include <vgui_controls/EditablePanel.h>
#include <FileSystem.h>
#include "vgui_controls/imagepanel.h"

#define PANEL_GRAPHICMENU "graphic_menu"

using namespace vgui;

class CCSBTEGraphicMenu : public CClassMenu
{
private:
	DECLARE_CLASS_SIMPLE(CCSBTEGraphicMenu, CClassMenu);

public:
	CCSBTEGraphicMenu(void);

public:
	void Setup(int flags);
	void SetTitle(const char *title);
	char *GetItemName(int slot);
	void ClearItems(void);
	void AddItem(int slot, const char *text, bool enabled);

public:
	virtual void PaintBackground(void);
	virtual void PerformLayout(void);
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

public:
	Panel *CreateControlByName(const char *controlName);
	const char *GetName(void);
	void ShowPanel(bool bShow);
	void Update(void);
	void Reset(void);
	void SetVisible(bool state);
	bool NeedsUpdate(void) { return true; }
};

#endif