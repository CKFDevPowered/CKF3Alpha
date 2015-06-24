#include <stdio.h>

#include "EngineInterface.h"
#include "DialogGameInfo.h"
#include "IGameList.h"
#include "ServerBrowserDialog.h"
#include "ServerBrowser.h"
#include "DialogServerPassword.h"
#include "VACBannedConnRefusedDialog.h"
#include "steam_api.h"

#include <vgui/ISystem.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <KeyValues.h>

#include <vgui_controls/Label.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/ToggleButton.h>
#include <vgui_controls/RadioButton.h>

using namespace vgui;

static const long RETRY_TIME = 10000;
static const long CHALLENGE_ENTRIES = 1024;

CDialogGameInfo::CDialogGameInfo(vgui::Panel *parent, int serverIP, int queryPort, unsigned short connectionPort) : Frame(parent, "DialogGameInfo"), m_CallbackPersonaStateChange(this, &CDialogGameInfo::OnPersonaStateChange)
{
	SetBounds(0, 0, 512, 512);
	SetMinimumSize(416, 340);
	SetDeleteSelfOnClose(true);

	m_bConnecting = false;
	m_bServerFull = false;
	m_bShowAutoRetryToggle = false;
	m_bServerNotResponding = false;
	m_bShowingExtendedOptions = false;
	m_SteamIDFriend = 0;
	m_hPingQuery = HSERVERQUERY_INVALID;
	m_hPlayersQuery = HSERVERQUERY_INVALID;
	m_bPlayerListUpdatePending = false;

	m_szPassword[0] = 0;

	m_pConnectButton = new Button(this, "Connect", "#ServerBrowser_JoinGame");
	m_pCloseButton = new Button(this, "Close", "#ServerBrowser_Close");
	m_pRefreshButton = new Button(this, "Refresh", "#ServerBrowser_Refresh");
	m_pInfoLabel = new Label(this, "InfoLabel", "");
	m_pAutoRetry = new ToggleButton(this, "AutoRetry", "#ServerBrowser_AutoRetry");
	m_pAutoRetry->AddActionSignalTarget(this);

	m_pAutoRetryAlert = new RadioButton(this, "AutoRetryAlert", "#ServerBrowser_AlertMeWhenSlotOpens");
	m_pAutoRetryJoin = new RadioButton(this, "AutoRetryJoin", "#ServerBrowser_JoinWhenSlotOpens");
	m_pPlayerList = new ListPanel(this, "PlayerList");
	m_pPlayerList->AddColumnHeader(0, "PlayerName", "#ServerBrowser_PlayerName", 156);
	m_pPlayerList->AddColumnHeader(1, "Score", "#ServerBrowser_Score", 64);
	m_pPlayerList->AddColumnHeader(2, "Time", "#ServerBrowser_Time", 64);

	m_pPlayerList->SetSortFunc(2, &PlayerTimeColumnSortFunc);

	PostMessage(m_pPlayerList, new KeyValues("SetSortColumn", "column", 2));
	PostMessage(m_pPlayerList, new KeyValues("SetSortColumn", "column", 1));
	PostMessage(m_pPlayerList, new KeyValues("SetSortColumn", "column", 1));

	m_pAutoRetryAlert->SetSelected(true);

	m_pConnectButton->SetCommand(new KeyValues("Connect"));
	m_pCloseButton->SetCommand(new KeyValues("Close"));
	m_pRefreshButton->SetCommand(new KeyValues("Refresh"));

	m_iRequestRetry = 0;

	memset(&m_Server, 0, sizeof(m_Server));
	m_Server.m_NetAdr.Init(serverIP, queryPort, connectionPort);

	RequestInfo();

	ivgui()->AddTickSignal(GetVPanel());

	LoadControlSettings("Servers/DialogGameInfo.res");
	RegisterControlSettingsFile("Servers/DialogGameInfo_SinglePlayer.res");
	RegisterControlSettingsFile("Servers/DialogGameInfo_AutoRetry.res");
}

CDialogGameInfo::~CDialogGameInfo(void)
{
	if (!SteamMatchmakingServers())
		return;

	if (m_hPingQuery != HSERVERQUERY_INVALID)
		SteamMatchmakingServers()->CancelServerQuery(m_hPingQuery);

	if (m_hPlayersQuery != HSERVERQUERY_INVALID)
		SteamMatchmakingServers()->CancelServerQuery(m_hPlayersQuery);
}

void CDialogGameInfo::SendPlayerQuery(uint32 unIP, uint16 usQueryPort)
{
	if (!SteamMatchmakingServers())
		return;

	if (m_hPlayersQuery != HSERVERQUERY_INVALID)
		SteamMatchmakingServers()->CancelServerQuery(m_hPlayersQuery);

	m_hPlayersQuery = SteamMatchmakingServers()->PlayerDetails(unIP, usQueryPort, this);
	m_bPlayerListUpdatePending = true;
}

void CDialogGameInfo::Run(const char *titleName)
{
	if (titleName)
		SetTitle("#ServerBrowser_GameInfoWithNameTitle", true);
	else
		SetTitle("#ServerBrowser_GameInfoWithNameTitle", true);

	SetDialogVariable("game", titleName);

	RequestInfo();
	Activate();
}

void CDialogGameInfo::ChangeGame(int serverIP, int queryPort, unsigned short connectionPort)
{
	memset(&m_Server, 0x0, sizeof(m_Server));

	m_Server.m_NetAdr.Init(serverIP, queryPort, connectionPort);

	int x, y;
	GetPos(x, y);

	if (!m_Server.m_NetAdr.GetIP() || !m_Server.m_NetAdr.GetQueryPort())
	{
		SetMinimumSize(0, 0);
		SetSizeable(false);
		LoadControlSettings("Servers/DialogGameInfo_SinglePlayer.res");
	}
	else
	{
		SetMinimumSize(416, 340);
		SetSizeable(true);
		LoadControlSettings("Servers/DialogGameInfo.res");
	}

	SetPos(x, y);

	m_iRequestRetry = 0;

	RequestInfo();
	InvalidateLayout();
}

void CDialogGameInfo::OnPersonaStateChange(PersonaStateChange_t *pPersonaStateChange)
{
	if (m_SteamIDFriend && m_SteamIDFriend == pPersonaStateChange->m_ulSteamID)
	{
		FriendGameInfo_t gameinfo;

		if (SteamFriends()->GetFriendGamePlayed(m_SteamIDFriend, &gameinfo))
		{
			if (pPersonaStateChange->m_nChangeFlags & k_EPersonaChangeGamePlayed)
				ChangeGame(gameinfo.m_unGameIP, gameinfo.m_usQueryPort, gameinfo.m_usGamePort);
		}
	}
}

void CDialogGameInfo::SetFriend(uint64 ulSteamIDFriend)
{
	FriendGameInfo_t gameinfo;

	if (SteamFriends()->GetFriendGamePlayed(ulSteamIDFriend, &gameinfo))
	{
		uint16 usConnPort = gameinfo.m_usGamePort;

		if (gameinfo.m_usQueryPort < QUERY_PORT_ERROR)
			usConnPort = gameinfo.m_usGamePort;

		ChangeGame(gameinfo.m_unGameIP, usConnPort, gameinfo.m_usGamePort);
	}

	m_SteamIDFriend = ulSteamIDFriend;

	SetTitle("#ServerBrowser_GameInfoWithNameTitle", true);

	SetDialogVariable("game", SteamFriends()->GetFriendPersonaName(ulSteamIDFriend));
	SetDialogVariable("friend", SteamFriends()->GetFriendPersonaName(ulSteamIDFriend));
}

uint64 CDialogGameInfo::GetAssociatedFriend(void)
{
	return m_SteamIDFriend;
}

void CDialogGameInfo::PerformLayout(void)
{
	BaseClass::PerformLayout();

	SetControlString("ServerText", m_Server.GetName());
	SetControlString("GameText", m_Server.m_szGameDescription);
	SetControlString("MapText", m_Server.m_szMap);

	if (!m_Server.m_bHadSuccessfulResponse)
		SetControlString("SecureText", "");
	else if (m_Server.m_bSecure)
		SetControlString("SecureText", "#ServerBrowser_Secure");
	else
		SetControlString("SecureText", "#ServerBrowser_NotSecure");

	char buf[128];

	if (m_Server.m_nMaxPlayers > 0)
	{
		Q_snprintf(buf, sizeof(buf), "%d / %d", m_Server.m_nPlayers, m_Server.m_nMaxPlayers);
	}
	else
	{
		buf[0] = 0;
	}

	SetControlString("PlayersText", buf);

	if (m_Server.m_NetAdr.GetIP() && m_Server.m_NetAdr.GetQueryPort())
	{
		SetControlString("ServerIPText", m_Server.m_NetAdr.GetConnectionAddressString());

		m_pConnectButton->SetEnabled(true);

		if (m_pAutoRetry->IsSelected())
		{
			m_pAutoRetryAlert->SetVisible(true);
			m_pAutoRetryJoin->SetVisible(true);
		}
		else
		{
			m_pAutoRetryAlert->SetVisible(false);
			m_pAutoRetryJoin->SetVisible(false);
		}
	}
	else
	{
		SetControlString("ServerIPText", "");
		m_pConnectButton->SetEnabled(false);
	}

	if (m_Server.m_bHadSuccessfulResponse)
	{
		Q_snprintf(buf, sizeof(buf), "%d", m_Server.m_nPing);
		SetControlString("PingText", buf);
	}
	else
	{
		SetControlString("PingText", "");
	}

	if (m_pAutoRetry->IsSelected())
	{
		if (m_Server.m_nPlayers < m_Server.m_nMaxPlayers)
		{
			m_pInfoLabel->SetText("#ServerBrowser_PressJoinToConnect");
		}
		else if (m_pAutoRetryJoin->IsSelected())
		{
			m_pInfoLabel->SetText("#ServerBrowser_JoinWhenSlotIsFree");
		}
		else
		{
			m_pInfoLabel->SetText("#ServerBrowser_AlertWhenSlotIsFree");
		}
	}
	else if (m_bServerFull)
	{
		m_pInfoLabel->SetText("#ServerBrowser_CouldNotConnectServerFull");
	}
	else if (m_bServerNotResponding)
	{
		m_pInfoLabel->SetText("#ServerBrowser_ServerNotResponding");
	}
	else
	{
		m_pInfoLabel->SetText("");
	}

	if (m_Server.m_bHadSuccessfulResponse && !(m_Server.m_nPlayers + m_Server.m_nBotPlayers))
	{
		m_pPlayerList->SetEmptyListText("#ServerBrowser_ServerHasNoPlayers");
	}
	else
	{
		m_pPlayerList->SetEmptyListText("#ServerBrowser_ServerNotResponding");
	}

	m_pAutoRetry->SetVisible(m_bShowAutoRetryToggle);

	Repaint();
}

void CDialogGameInfo::Connect(void)
{
	OnConnect();
}

void CDialogGameInfo::OnConnect(void)
{
	m_bConnecting = true;

	m_bServerFull = false;
	m_bServerNotResponding = false;

	InvalidateLayout();

	m_iRequestRetry = 0;
	RequestInfo();
}

void CDialogGameInfo::OnConnectToGame(int ip, int port)
{
	if (m_Server.m_NetAdr.GetIP() == (uint32)ip && m_Server.m_NetAdr.GetConnectionPort() == (uint16)port)
	{
		Close();
	}
}

void CDialogGameInfo::OnRefresh(void)
{
	m_iRequestRetry = 0;

	RequestInfo();
}

void CDialogGameInfo::OnButtonToggled(Panel *panel)
{
	if (panel == m_pAutoRetry)
		ShowAutoRetryOptions(m_pAutoRetry->IsSelected());

	InvalidateLayout();
}

void CDialogGameInfo::ShowAutoRetryOptions(bool state)
{
	int growSize = 60;

	if (!state)
		growSize = -growSize;

	int x, y, wide, tall;
	GetBounds(x, y, wide, tall);
	SetMinimumSize(416, 340);

	if (state)
		LoadControlSettings("Servers/DialogGameInfo_AutoRetry.res");
	else
		LoadControlSettings("Servers/DialogGameInfo.res");

	SetBounds(x, y, wide, tall + growSize);
	PerformLayout();

	m_pAutoRetryAlert->SetSelected(true);

	InvalidateLayout();
}

void CDialogGameInfo::RequestInfo(void)
{
	if (!SteamMatchmakingServers())
		return;

	if (m_iRequestRetry == 0)
	{
		m_iRequestRetry = system()->GetTimeMillis() + RETRY_TIME;

		if (m_hPingQuery != HSERVERQUERY_INVALID)
			SteamMatchmakingServers()->CancelServerQuery(m_hPingQuery);

		m_hPingQuery = SteamMatchmakingServers()->PingServer(m_Server.m_NetAdr.GetIP(), m_Server.m_NetAdr.GetQueryPort(), this);
	}
}

void CDialogGameInfo::OnTick(void)
{
	if (m_iRequestRetry && m_iRequestRetry < system()->GetTimeMillis())
	{
		m_iRequestRetry = 0;
		RequestInfo();
	}
}

void CDialogGameInfo::ServerResponded(gameserveritem_t &server)
{
	m_hPingQuery = HSERVERQUERY_INVALID;
	m_Server = server;

	if (m_bConnecting)
	{
		ConnectToServer();
	}
	else if (m_pAutoRetry->IsSelected() && server.m_nPlayers < server.m_nMaxPlayers)
	{
		surface()->PlaySound("servers/game_ready.wav");

		FlashWindow();

		if (m_pAutoRetryJoin->IsSelected())
		{
			ConnectToServer();
		}
	}
	else
	{
		SendPlayerQuery(server.m_NetAdr.GetIP(), server.m_NetAdr.GetQueryPort());
	}

	m_bServerNotResponding = false;

	InvalidateLayout();
	Repaint();
}

void CDialogGameInfo::ServerFailedToRespond(void)
{
	if (!m_Server.m_bHadSuccessfulResponse)
		m_bServerNotResponding = true;

	InvalidateLayout();
	Repaint();
}

void CDialogGameInfo::ApplyConnectCommand(const gameserveritem_t &server)
{
	char command[256];

	if (m_szPassword[0])
	{
		Q_snprintf(command, Q_ARRAYSIZE(command), "password \"%s\"\n", m_szPassword);
		engine->pfnClientCmd(command);
	}

	Q_snprintf(command, Q_ARRAYSIZE(command), "connect %s\n", server.m_NetAdr.GetConnectionAddressString());
	engine->pfnClientCmd(command);
}

void CDialogGameInfo::ConstructConnectArgs(char *pchOptions, int cchOptions, const gameserveritem_t &server)
{
	Q_snprintf(pchOptions, cchOptions, " +connect %s", server.m_NetAdr.GetConnectionAddressString());

	if (m_szPassword[0])
	{
		Q_strncat(pchOptions, " +password \"", cchOptions);
		Q_strncat(pchOptions, m_szPassword, cchOptions);
		Q_strncat(pchOptions, "\"", cchOptions);
	}
}

void CDialogGameInfo::ConnectToServer(void)
{
	m_bConnecting = false;

	if (m_Server.m_bSecure && ServerBrowser().IsVACBannedFromGame(m_Server.m_nAppID))
	{
		CVACBannedConnRefusedDialog *pDlg = new CVACBannedConnRefusedDialog(GetVParent(), "VACBannedConnRefusedDialog");
		pDlg->Activate();
		Close();
		return;
	}

	if (m_Server.m_bPassword && !m_szPassword[0])
	{
		CDialogServerPassword *box = new CDialogServerPassword(this);
		box->AddActionSignalTarget(this);
		box->Activate(m_Server.GetName(), 0);
		return;
	}

	if (m_Server.m_nPlayers >= m_Server.m_nMaxPlayers)
	{
		m_bServerFull = true;
		m_bShowAutoRetryToggle = true;

		InvalidateLayout();
		return;
	}

	ApplyConnectCommand(m_Server);

	PostMessage(this, new KeyValues("Close"));
}

void CDialogGameInfo::RefreshComplete(EMatchMakingServerResponse response)
{
}

void CDialogGameInfo::OnJoinServerWithPassword(const char *password)
{
	Q_strncpy(m_szPassword, password, sizeof(m_szPassword));

	OnConnect();
}

void CDialogGameInfo::ClearPlayerList(void)
{
	m_pPlayerList->DeleteAllItems();

	Repaint();
}

void CDialogGameInfo::AddPlayerToList(const char *playerName, int score, float timePlayedSeconds)
{
	if (m_bPlayerListUpdatePending)
	{
		m_bPlayerListUpdatePending = false;
		m_pPlayerList->RemoveAll();
	}

	KeyValues *player = new KeyValues("player");
	player->SetString("PlayerName", playerName);
	player->SetInt("Score", score);
	player->SetInt("TimeSec", (int)timePlayedSeconds);

	int seconds = (int)timePlayedSeconds;
	int minutes = seconds / 60;
	int hours = minutes / 60;

	seconds %= 60;
	minutes %= 60;

	char buf[64];
	buf[0] = 0;

	if (hours)
		Q_snprintf(buf, sizeof(buf), "%dh %dm %ds", hours, minutes, seconds);
	else if (minutes)
		Q_snprintf(buf, sizeof(buf), "%dm %ds", minutes, seconds);
	else
		Q_snprintf(buf, sizeof(buf), "%ds", seconds);

	player->SetString("Time", buf);

	m_pPlayerList->AddItem(player, 0, false, true);
	player->deleteThis();
}

int CDialogGameInfo::PlayerTimeColumnSortFunc(ListPanel *pPanel, const ListPanelItem &p1, const ListPanelItem &p2)
{
	int p1time = p1.kv->GetInt("TimeSec");
	int p2time = p2.kv->GetInt("TimeSec");

	if (p1time > p2time)
		return -1;
	if (p1time < p2time)
		return 1;

	return 0;
}