#ifndef SPECTATORGUI_H
#define SPECTATORGUI_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/IScheme.h>
#include <vgui/keycode.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/ComboBox.h>

#define PANEL_SPECGUI "specgui"
#define PANEL_SPECMENU "specmenu"

class KeyValues;
class CommandMenu;

namespace vgui
{
	class TextEntry;
	class Button;
	class Panel;
	class ImagePanel;
	class ComboBox;
}

#define BLACK_BAR_COLOR Color(0, 0, 0, 196)

class CSpectatorGUI : public vgui::EditablePanel, public CViewPortPanel
{
	DECLARE_CLASS_SIMPLE(CSpectatorGUI, vgui::EditablePanel);

public:
	CSpectatorGUI(void);
	virtual ~CSpectatorGUI(void);

public:
	virtual const char *GetName(void) { return PANEL_SPECGUI; }
	virtual void SetData(KeyValues *data) {}
	virtual void Reset(void) {};
	virtual void Update(void);
	virtual bool NeedsUpdate(void);
	virtual bool HasInputElements(void) { return false; }
	virtual void ShowPanel(bool bShow);

public:
	DECLARE_VIEWPORT_PANEL_SIMPLE();

public:
	virtual void Paint(void);
	virtual void OnThink(void);
	virtual int GetTopBarHeight(void) { return m_pTopBar->GetTall(); }
	virtual int GetBottomBarHeight(void) { return m_pBottomBarBlank->GetTall(); }
	virtual bool ShouldShowPlayerLabel(int specmode);
	virtual Color GetBlackBarColor(void) { return BLACK_BAR_COLOR; }

public:
	void SetLabelText(const char *textEntryName, const char *text);
	void SetLabelText(const char *textEntryName, wchar_t *text);
	void MoveLabelToFront(const char *textEntryName);
	void UpdateTimer(void);
	void SetLogoImage(const char *image);

protected:
	enum { INSET_OFFSET = 2 };

protected:
	virtual void PerformLayout(void);
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

protected:
	vgui::Panel *m_pTopBar;
	vgui::Panel *m_pBottomBarBlank;

	vgui::ImagePanel *m_pBannerImage;
	vgui::Label *m_pPlayerLabel;

	bool m_bSpecScoreboard;
	bool m_bLastSpecPic;

	bool m_bHelpShown;
};

class CSpectatorMenu : public vgui::Frame, public CViewPortPanel
{
	DECLARE_CLASS_SIMPLE(CSpectatorMenu, vgui::Frame);

public:
	CSpectatorMenu(void);
	~CSpectatorMenu(void) {}

	virtual const char *GetName(void) { return PANEL_SPECMENU; }
	virtual void SetData(KeyValues *data) {};
	virtual void Reset(void) { m_pPlayerList->DeleteAllItems(); }
	virtual void Update(void);
	virtual bool NeedsUpdate(void) { return true; }
	virtual bool HasInputElements(void) { return true; }
	virtual void ShowPanel(bool bShow);

public:
	DECLARE_VIEWPORT_PANEL_SIMPLE();

private:
	MESSAGE_FUNC_PARAMS(OnTextChanged, "TextChanged", data);

private:
	virtual void OnCommand(const char *command);
	virtual void OnKeyCodePressed(vgui::KeyCode code);
	virtual void OnKeyCodeReleased(vgui::KeyCode code);
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void PerformLayout(void);

private:
	void SetViewModeText(const char *text) { m_pViewOptions->SetText(text); }
	void SetPlayerFgColor(Color c1) { m_pPlayerList->SetFgColor(c1); }

private:
	vgui::ComboBox *m_pPlayerList;
	vgui::ComboBox *m_pViewOptions;
	vgui::ComboBox *m_pConfigSettings;

	vgui::Button *m_pLeftButton;
	vgui::Button *m_pRightButton;

	vgui::KeyCode m_iDuckKey;
	bool m_bDuckPressed;

	CommandMenu *m_pSpectatorMenu;
	CommandMenu *m_pSpectatorModes;
};

extern CSpectatorGUI *g_pSpectatorGUI;
extern CSpectatorMenu *g_pSpectatorMenu;

#endif
