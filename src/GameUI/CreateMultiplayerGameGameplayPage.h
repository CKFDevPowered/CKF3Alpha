#ifndef CREATEMULTIPLAYERGAMEGAMEPLAYPAGE_H
#define CREATEMULTIPLAYERGAMEGAMEPLAYPAGE_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyPage.h>

class CPanelListPanel;
class CDescription;
class mpcontrol_t;

class CCreateMultiplayerGameGameplayPage : public vgui::PropertyPage
{
public:
	CCreateMultiplayerGameGameplayPage(vgui::Panel *parent, const char *name);
	~CCreateMultiplayerGameGameplayPage(void);

public:
	int GetMaxPlayers(void);
	const char *GetPassword(void);
	const char *GetHostName(void);
	void SaveValues(void);

private:
	const char *GetValue(const char *cvarName, const char *defaultValue);
	void LoadGameOptionsList(void);
	void GatherCurrentValues(void);

public:
	CDescription *m_pDescription;
	mpcontrol_t *m_pList;
	CPanelListPanel *m_pOptionsList;
};

#endif
