#ifndef CSTRIKEBUYMENU_H
#define CSTRIKEBUYMENU_H

#ifdef _WIN32
#pragma once
#endif

#define PANEL_BUY_CT "buy_ct"
#define PANEL_BUY_TER "buy_ter"

#include <vgui_controls/WizardPanel.h>
#include "game_controls\buymenu.h"

class BuyPresetEditPanel;
class BuyPresetButton;

namespace vgui
{
	class Panel;
	class Button;
	class Label;
}

enum
{
	NUM_BUY_PRESET_BUTTONS = 5,
};

class CCSBaseBuyMenu : public CBuyMenu
{
	typedef CBuyMenu BaseClass;

public:
	CCSBaseBuyMenu(const char *subPanelName);

public:
	virtual void Init(void);
	virtual void VidInit(void);
	virtual void ShowPanel(bool bShow);
	virtual void Paint(void);
	virtual void SetVisible(bool state);
	virtual void GotoMenu(int iMenu);
	virtual void ActivateMenu(int iMenu);

public:
	virtual void PaintBackground(void);
	virtual void PerformLayout(void);
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
};

class CCSBuyMenu_CT : public CCSBaseBuyMenu
{
private:
	typedef vgui::WizardPanel BaseClass;

public:
	CCSBuyMenu_CT(void);

public:
	virtual const char *GetName(void) { return PANEL_BUY_CT; }
};

class CCSBuyMenu_TER : public CCSBaseBuyMenu
{
private:
	typedef vgui::WizardPanel BaseClass;

public:
	CCSBuyMenu_TER(void);

public:
	virtual const char *GetName(void) { return PANEL_BUY_TER; }
};

#endif