#ifndef CVARNEGATECHECKBUTTON_H
#define CVARNEGATECHECKBUTTON_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/CheckButton.h>

class CCvarNegateCheckButton : public vgui::CheckButton
{
	DECLARE_CLASS_SIMPLE(CCvarNegateCheckButton, vgui::CheckButton);

public:
	CCvarNegateCheckButton(vgui::Panel *parent, const char *panelName, const char *text, char const *cvarname);
	~CCvarNegateCheckButton(void);

public:
	virtual void SetSelected(bool state);
	virtual void Paint(void);

public:
	void Reset(void);
	void ApplyChanges(void);
	bool HasBeenModified(void);

private:
	MESSAGE_FUNC(OnButtonChecked, "CheckButtonChecked");

private:
	char *m_pszCvarName;
	bool m_bStartState;
};

#endif