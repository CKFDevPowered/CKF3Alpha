#ifndef DIALOGADDSERVER_H
#define DIALOGADDSERVER_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/ListPanel.h>
#include "netadr.h"
#include "isteammatchmaking.h"

class CBaseGamesPage;
class IGameList;

class CAddServerGameList : public vgui::ListPanel
{
public:
	DECLARE_CLASS_SIMPLE(CAddServerGameList, vgui::ListPanel);

public:
	CAddServerGameList(vgui::Panel *parent, const char *panelName);

public:
	virtual void OnKeyCodeTyped(vgui::KeyCode code);
};

class CDialogAddServer : public vgui::Frame, public ISteamMatchmakingPingResponse
{
public:
	DECLARE_CLASS_SIMPLE(CDialogAddServer, vgui::Frame);

public:
	friend class CAddServerGameList;

public:
	CDialogAddServer(vgui::Panel *parent, IGameList *gameList);
	~CDialogAddServer(void);

public:
	void ServerResponded(gameserveritem_t &server);
	void ServerFailedToRespond(void);
	void TestServers(void);

public:
	void ApplySchemeSettings(vgui::IScheme *pScheme);

public:
	MESSAGE_FUNC(OnItemSelected, "ItemSelected");

private:
	void OnCommand(const char *command);
	void OnOK(void);
	void OnKeyCodeTyped(vgui::KeyCode code);

	MESSAGE_FUNC(OnTextChanged, "TextChanged");

private:
	IGameList *m_pGameList;

	vgui::Button *m_pTestServersButton;
	vgui::Button *m_pAddServerButton;
	vgui::Button *m_pAddSelectedServerButton;

	vgui::PropertySheet *m_pTabPanel;
	vgui::TextEntry *m_pTextEntry;
	CAddServerGameList *m_pDiscoveredGames;
	int m_OriginalHeight;
	CUtlVector<gameserveritem_t> m_Servers;
	CUtlVector<HServerQuery> m_Queries;
};

#endif