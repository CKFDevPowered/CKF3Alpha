#ifndef CREATEMULTIPLAYERGAMESERVERPAGE_H
#define CREATEMULTIPLAYERGAMESERVERPAGE_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyPage.h>
#include "cvartogglecheckbutton.h"

class CCreateMultiplayerGameServerPage : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(CCreateMultiplayerGameServerPage, vgui::PropertyPage);

public:
	CCreateMultiplayerGameServerPage(vgui::Panel *parent, const char *name);
	~CCreateMultiplayerGameServerPage(void);

public:
	void SetMap(const char *name);
	bool IsRandomMapSelected(void);
	const char *GetMapName(void);

public:
	void EnableBots(KeyValues *data);
	int GetBotQuota(void);
	bool GetBotsEnabled(void);

protected:
	virtual void OnApplyChanges(void);

protected:
	MESSAGE_FUNC(OnCheckButtonChecked, "CheckButtonChecked");

public:
	char *GetBOTCommandBuffer(void);

private:
	void LoadMapList(void);
	void LoadMaps(const char *pszPathID);

public:
	static int MapListCompare(const unsigned long *a1, const unsigned long *a2);

private:
	vgui::ComboBox *m_pMapList;
	vgui::CheckButton *m_pEnableBotsCheck;
	CCvarToggleCheckButton *m_pEnableTutorCheck;
	KeyValues *m_pSavedData;

	enum { DATA_STR_LENGTH = 64 };

	char m_szHostName[DATA_STR_LENGTH];
	char m_szPassword[DATA_STR_LENGTH];
	char m_szMapName[DATA_STR_LENGTH];
	int m_iMaxPlayers;

	CUtlVector<unsigned long> m_vMapCache;
};

#endif
