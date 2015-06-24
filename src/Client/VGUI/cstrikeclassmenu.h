#ifndef CSCLASSMENU_H
#define CSCLASSMENU_H

#ifdef _WIN32
#pragma once
#endif

#include "game_controls\classmenu.h"
#include <vgui_controls/EditablePanel.h>
#include <FileSystem.h>
#include "vgui_controls/imagepanel.h"

#define PANEL_CLASS_CT "class_ct"
#define PANEL_CLASS_TER "class_ter"

using namespace vgui;

class CCSClassMenu_TER : public CClassMenu
{
private:
	DECLARE_CLASS_SIMPLE(CCSClassMenu_TER, CClassMenu);

public:
	CCSClassMenu_TER(void);

public:
	virtual void PaintBackground(void);
	virtual void PerformLayout(void);
	virtual void ApplySchemeSettings(IScheme *pScheme);

protected:
	virtual MouseOverPanelButton *CreateNewMouseOverPanelButton(EditablePanel *panel);

public:
	const char *GetName(void);
	void ShowPanel(bool bShow);
	void Update(void);
	void Reset(void);
	void SetVisible(bool state);
	bool NeedsUpdate(void) { return true; }
};

class CCSClassMenu_CT : public CClassMenu
{
private:
	DECLARE_CLASS_SIMPLE(CCSClassMenu_CT, CClassMenu);

public:
	CCSClassMenu_CT(void);

public:
	virtual void PaintBackground(void);
	virtual void PerformLayout(void);
	virtual void ApplySchemeSettings(IScheme *pScheme);

protected:
	virtual MouseOverPanelButton *CreateNewMouseOverPanelButton(EditablePanel *panel);

public:
	const char *GetName(void);
	void ShowPanel(bool bShow);
	void Update(void);
	void Reset(void);
	void SetVisible(bool state);
	bool NeedsUpdate(void) { return true; }
};

#endif