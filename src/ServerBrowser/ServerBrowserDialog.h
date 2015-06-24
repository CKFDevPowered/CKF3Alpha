#ifndef SERVERBROWSERDIALOG_H
#define SERVERBROWSERDIALOG_H

#ifdef _WIN32
#pragma once
#endif

#pragma warning(disable: 4355)

#include <vgui_controls/Frame.h>
#include <vgui_controls/ListPanel.h>
#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/PHandle.h>

#include "UtlVector.h"
#include "netadr.h"
#include "isteammatchmaking.h"
#include "IGameList.h"

class CServerContextMenu;

class CFavoriteGames;
class CInternetGames;
class CSpectateGames;
class CLanGames;
class CHistoryGames;
class CFriendsGames;
class CCustomGames;
class CDialogGameInfo;
class CBaseGamesPage;

void GetMostCommonQueryPorts(CUtlVector<uint16> &ports);

class CServerBrowserDialog : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CServerBrowserDialog, vgui::Frame); 

public:
	CServerBrowserDialog(vgui::Panel *parent);
	~CServerBrowserDialog(void);

public:
	void Initialize(void);
	void Open(void);
	gameserveritem_t *GetServer(unsigned int serverID);
	void UpdateStatusText(const char *format, ...);
	void UpdateStatusText(wchar_t *unicode);
	CServerContextMenu *GetContextMenu(vgui::Panel *pParent);
	static CServerBrowserDialog *GetInstance(void);
	void AddServerToFavorites(gameserveritem_t &server);
	CDialogGameInfo *JoinGame(IGameList *gameList, unsigned int serverIndex);
	CDialogGameInfo *JoinGame(int serverIP, int serverPort);
	CDialogGameInfo *OpenGameInfoDialog(IGameList *gameList, unsigned int serverIndex);
	CDialogGameInfo *OpenGameInfoDialog(int serverIP, uint16 connPort, uint16 queryPort);
	void CloseAllGameInfoDialogs(void);
	CDialogGameInfo *GetDialogGameInfoForFriend(uint64 ulSteamIDFriend);
	KeyValues *GetFilterSaveData(const char *filterSet);
	const char *GetActiveModName(void);
	int GetActiveAppID(void);
	const char *GetActiveGameName(void);
	void LoadUserData(void);
	void SaveUserData(void);
	void RefreshCurrentPage(void);
	gameserveritem_t *GetCurrentConnectedServer(void) { return &m_CurrentConnection; }

private:
	MESSAGE_FUNC(OnTick, "Tick");
	MESSAGE_FUNC(OnGameListChanged, "PageChanged");
	MESSAGE_FUNC_PARAMS(OnActiveGameName, "ActiveGameName", name);
	MESSAGE_FUNC_PARAMS(OnConnectToGame, "ConnectedToGame", kv);
	MESSAGE_FUNC(OnDisconnectFromGame, "DisconnectedFromGame");

private:
	void ReloadFilterSettings(void);

	virtual bool GetDefaultScreenPosition(int &x, int &y, int &wide, int &tall);
	virtual void ActivateBuildMode(void);

private:
	CUtlVector<vgui::DHANDLE<CDialogGameInfo>> m_GameInfoDialogs;

	IGameList *m_pGameList;
	vgui::Label *m_pStatusLabel;

	vgui::PropertySheet *m_pTabPanel;
	CFavoriteGames *m_pFavorites;
	CHistoryGames *m_pHistory;
	CInternetGames *m_pInternetGames;
	CSpectateGames *m_pSpectateGames;
	CLanGames *m_pLanGames;
	CFriendsGames *m_pFriendsGames;

	KeyValues *m_pSavedData;
	KeyValues *m_pFilterData;

	CServerContextMenu *m_pContextMenu;

	char m_szGameName[128];
	char m_szModDir[128];
	int m_iLimitAppID;

	bool m_bCurrentlyConnected;
	gameserveritem_t m_CurrentConnection;
};

extern CServerBrowserDialog &ServerBrowserDialog(void);

#endif