#ifndef DIALOGGAMEINFO_H
#define DIALOGGAMEINFO_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/ListPanel.h>
#include "netadr.h"
#include "isteammatchmaking.h"
#include "steam_api.h"

class CDialogGameInfo : public vgui::Frame, public ISteamMatchmakingPlayersResponse, public ISteamMatchmakingPingResponse
{
	DECLARE_CLASS_SIMPLE(CDialogGameInfo, vgui::Frame); 

public:
	CDialogGameInfo(vgui::Panel *parent, int serverIP, int queryPort, unsigned short connectionPort);
	~CDialogGameInfo(void);

public:
	void Run(const char *titleName);
	void ChangeGame(int serverIP, int queryPort, unsigned short connectionPort);
	void SetFriend(uint64 ulSteamIDFriend);
	uint64 GetAssociatedFriend(void);
	void Connect(void);

private:
	STEAM_CALLBACK(CDialogGameInfo, OnPersonaStateChange, PersonaStateChange_t, m_CallbackPersonaStateChange);

public:
	virtual void ServerResponded(gameserveritem_t &server);
	virtual void ServerFailedToRespond(void);
	virtual void AddPlayerToList(const char *playerName, int score, float timePlayedSeconds);
	virtual void PlayersFailedToRespond(void) {}
	virtual void PlayersRefreshComplete(void) { m_hPlayersQuery = HSERVERQUERY_INVALID; }
	virtual void RefreshComplete(EMatchMakingServerResponse response);
	virtual void ClearPlayerList(void);
	virtual void SendPlayerQuery(uint32 unIP, uint16 usQueryPort);

protected:
	MESSAGE_FUNC(OnConnect, "Connect");
	MESSAGE_FUNC(OnRefresh, "Refresh");
	MESSAGE_FUNC_PTR(OnButtonToggled, "ButtonToggled", panel);
	MESSAGE_FUNC_PTR(OnRadioButtonChecked, "RadioButtonChecked", panel)
	{
		OnButtonToggled(panel);
	}

	MESSAGE_FUNC_CHARPTR(OnJoinServerWithPassword, "JoinServerWithPassword", password);
	MESSAGE_FUNC_INT_INT(OnConnectToGame, "ConnectedToGame", ip, port);

protected:
	virtual void OnTick(void);
	virtual void PerformLayout(void);

private:
	long m_iRequestRetry;

private:
	static int PlayerTimeColumnSortFunc(vgui::ListPanel *pPanel, const vgui::ListPanelItem &p1, const vgui::ListPanelItem &p2);

private:
	void RequestInfo(void);
	void ConnectToServer(void);
	void ShowAutoRetryOptions(bool state);
	void ConstructConnectArgs(char *pchOptions, int cchOptions, const gameserveritem_t &server);
	void ApplyConnectCommand(const gameserveritem_t &server);

private:
	vgui::Button *m_pConnectButton;
	vgui::Button *m_pCloseButton;
	vgui::Button *m_pRefreshButton;
	vgui::Label *m_pInfoLabel;
	vgui::ToggleButton *m_pAutoRetry;
	vgui::RadioButton *m_pAutoRetryAlert;
	vgui::RadioButton *m_pAutoRetryJoin;
	vgui::ListPanel *m_pPlayerList;

	enum { PING_TIMES_MAX = 4 };

	bool m_bConnecting;
	char m_szPassword[64];

	bool m_bServerNotResponding;
	bool m_bServerFull;
	bool m_bShowAutoRetryToggle;
	bool m_bShowingExtendedOptions;

	uint64 m_SteamIDFriend;

	gameserveritem_t m_Server;
	HServerQuery m_hPingQuery;
	HServerQuery m_hPlayersQuery;
	bool m_bPlayerListUpdatePending;
};

#endif