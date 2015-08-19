#ifndef CLASSMENU_H
#define CLASSMENU_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/HTML.h>
#include <UtlVector.h>
#include <vgui/ILocalize.h>
#include <vgui/KeyCode.h>
#include "mouseoverpanelbutton.h"

#define PANEL_CLASS "class"

namespace vgui
{
	class TextEntry;
}

class TeamFortressViewport;

class CClassMenu : public vgui::Frame, public CViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE(CClassMenu, vgui::Frame);

public:
	CClassMenu(void);
	CClassMenu(const char *panelName);
	virtual ~CClassMenu(void);

public:
	virtual void Init(void) {}
	virtual void VidInit(void) {}
	virtual void Reset(void);
	virtual void Update(void) {}

public:
	virtual const char *GetName(void) { return PANEL_CLASS; }
	virtual void SetData(KeyValues *data);
	virtual bool NeedsUpdate(void) { return false; }
	virtual bool HasInputElements(void) { return true; }
	virtual void ShowPanel(bool bShow);
	virtual bool IsDynamic(void) { return true; }

public:
	DECLARE_VIEWPORT_PANEL_SIMPLE();

protected:
	virtual vgui::Panel *CreateControlByName(const char *controlName);
	virtual MouseOverPanelButton *CreateNewMouseOverPanelButton(vgui::EditablePanel *panel);
	virtual void OnKeyCodePressed(vgui::KeyCode code);

protected:
	void SetLabelText(const char *textEntryName, const char *text);
	void SetEnableButton(const char *textEntryName, bool state);
	void SetVisibleButton(const char *textEntryName, bool state);

protected:
	void OnCommand(const char *command);

protected:
	vgui::KeyCode m_iScoreBoardKey;
	int m_iTeam;
	vgui::EditablePanel *m_pPanel;
	CUtlVector<MouseOverPanelButton *> m_mouseoverButtons;
};

#endif