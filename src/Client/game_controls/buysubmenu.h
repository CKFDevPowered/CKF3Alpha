#ifndef BUYSUBMENU_H
#define BUYSUBMENU_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/WizardSubPanel.h>
#include <vgui_controls/Button.h>
#include <UtlVector.h>
#include "game_controls/MouseOverPanelButton.h"

class CBuyMenu;

class CBuySubMenu : public vgui::WizardSubPanel
{
private:
	DECLARE_CLASS_SIMPLE(CBuySubMenu, vgui::WizardSubPanel);

public:
	CBuySubMenu(vgui::Panel *parent, const char *name = "BuySubMenu");
	~CBuySubMenu(void);

public:
	virtual void SetVisible(bool state);
	virtual void DeleteSubPanels(void);

public:
	virtual void OnCommand(const char *command);

public:
	virtual void Close(void);
	virtual void GotoNextSubPanel(void);
	virtual void SetupNextSubPanel(const char *fileName);

protected:
	virtual void SetNextSubPanel(vgui::WizardSubPanel *panel);
	virtual vgui::WizardSubPanel *GetNextSubPanel(void);
	virtual vgui::Panel *CreateControlByName(const char *controlName);
	virtual CBuySubMenu *CreateNewSubMenu(void);
	virtual MouseOverPanelButton *CreateNewMouseOverPanelButton(vgui::EditablePanel *panel);

protected:
	typedef struct
	{
		char filename[_MAX_PATH];
		CBuySubMenu *panel;
	}
	SubMenuEntry_t;

protected:
	vgui::EditablePanel *m_pPanel;
	MouseOverPanelButton *m_pFirstButton;
	CUtlVector<SubMenuEntry_t> m_SubMenus;
	vgui::WizardSubPanel *m_NextPanel;
};

#endif