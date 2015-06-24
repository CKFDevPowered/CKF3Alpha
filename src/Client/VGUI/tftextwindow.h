#ifndef TFTEXTWINDOW_H
#define TFTEXTWINDOW_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui/KeyCode.h>
#include <UtlVector.h>

#include <game_controls/vguitextwindow.h>
#include <game_controls/TFRichText.h>

namespace vgui
{
	class Label;
	class RichText;
	class Button;
	class TFRichText;
}

class CTFTextWindow : public CTextWindow
{
private:
	DECLARE_CLASS_SIMPLE(CTFTextWindow, CTextWindow);

public:
	CTFTextWindow(void);
	virtual ~CTFTextWindow(void);

public:
	virtual void Init(void);
	virtual bool NeedsUpdate(void) { return false; }
	virtual bool HasInputElements(void) { return true; }
	virtual bool IsDynamic(void) { return true; }

	virtual void OnCommand( const char *command );
	virtual void ShowText(const char *text);
	virtual void ShowPanel(bool bShow);
	virtual void Update(void);

	void ShowMOTD(char *title, char *string);

public:

protected:
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void OnKeyCodePressed(vgui::KeyCode code);

protected:
	vgui::TFRichText *m_pMessageLabel;
};

#endif