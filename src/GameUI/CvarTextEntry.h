#ifndef CVARTEXTENTRY_H
#define CVARTEXTENTRY_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/TextEntry.h>

class CCvarTextEntry : public vgui::TextEntry
{
	DECLARE_CLASS_SIMPLE(CCvarTextEntry, vgui::TextEntry);

public:
	CCvarTextEntry(vgui::Panel *parent, const char *panelName, char const *cvarname);
	~CCvarTextEntry(void);

public:
	MESSAGE_FUNC(OnTextChanged, "TextChanged");

public:
	void ApplyChanges(void);
	void ApplySchemeSettings(vgui::IScheme *pScheme);
	void Reset(void);
	bool HasBeenModified(void);

private:
	char *m_pszCvarName;
	char m_pszStartValue[64];
};

#endif
