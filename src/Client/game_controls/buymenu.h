#ifndef BUYMENU_H
#define BUYMENU_H

#ifdef _WIN32
#pragma once
#endif

#define PANEL_BUY "buy"

#include <vgui_controls/WizardPanel.h>

class CBuySubMenu;

namespace vgui
{
	class Panel;
}

class CBuyMenu : public vgui::WizardPanel, public CViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE(CBuyMenu, vgui::WizardPanel);

public:
	CBuyMenu(void);
	~CBuyMenu(void);

public:
	virtual void Init(void);
	virtual void VidInit(void);
	virtual const char *GetName(void) { return PANEL_BUY; }
	virtual void SetData(KeyValues *data) {}
	virtual void Reset(void) {}
	virtual void Update(void);
	virtual bool NeedsUpdate(void) { return false; }
	virtual bool HasInputElements(void) { return true; }
	virtual void ShowPanel(bool bShow);

public:
	DECLARE_VIEWPORT_PANEL_SIMPLE();

public:
	virtual void OnClose(void);

protected:
	CBuySubMenu *m_pMainMenu;

	int m_iTeam;
	int m_iClass;
};

#endif