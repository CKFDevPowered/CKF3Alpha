#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <time.h>

#include "EngineInterface.h"

#include <vgui/IInput.h>
#include <vgui/ISurface.h>
#include <vgui/IScheme.h>
#include <vgui/IVGui.h>
#include <vgui/ISystem.h>
#include <KeyValues.h>
#include <vgui/MouseCode.h>
#include "FileSystem.h"

#include <vgui_controls/Button.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/FocusNavGroup.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/ListPanel.h>
#include <vgui_controls/MessageBox.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/QueryBox.h>
#include <vgui_controls/AnimationController.h>

#include "ServerContextMenu.h"
#include "ServerBrowserDialog.h"
#include "DialogGameInfo.h"
#include "steam_api.h"

#include "InternetGames.h"
#include "FavoriteGames.h"
#include "SpectateGames.h"
#include "LanGames.h"
#include "HistoryGames.h"
#include "FriendsGames.h"

using namespace vgui;

static CServerBrowserDialog *s_InternetDlg = NULL;

CServerBrowserDialog &ServerBrowserDialog(void)
{
	return *CServerBrowserDialog::GetInstance();
}

void GetMostCommonQueryPorts(CUtlVector<uint16> &ports)
{
	for (int i = 0; i <= 5; i++)
	{
		ports.AddToTail(27015 + i);
		ports.AddToTail(26900 + i);
	}
#ifdef _DEBUG
	ports.AddToTail(4242);
#endif
}

CServerBrowserDialog::CServerBrowserDialog(vgui::Panel *parent) : Frame(parent, "CServerBrowserDialog")
{
	s_InternetDlg = this;

	m_szGameName[0] = 0;
	m_szModDir[0] = 0;
	m_iLimitAppID = 70;
	m_pSavedData = NULL;
	m_pFilterData = NULL;
	m_pInternetGames = NULL;
	m_pFavorites = NULL;
	m_pHistory = NULL;
	m_pSpectateGames = NULL;
	m_pLanGames = NULL;

	LoadUserData();

	m_pInternetGames = new CInternetGames(this);
	m_pFavorites = new CFavoriteGames(this);
	m_pHistory = new CHistoryGames(this);
	m_pSpectateGames = new CSpectateGames(this);
	m_pLanGames = new CLanGames(this);
	m_pFriendsGames = new CFriendsGames(this);

	m_pInternetGames->MakeReadyForUse();
	m_pFavorites->MakeReadyForUse();
	m_pHistory->MakeReadyForUse();
	m_pSpectateGames->MakeReadyForUse();
	m_pLanGames->MakeReadyForUse();
	m_pFriendsGames->MakeReadyForUse();

	SetMinimumSize(640, 384);
	SetSize(640, 384);

	m_pContextMenu = new CServerContextMenu(this);
	m_pContextMenu->SetVisible(false);
	m_pStatusLabel = new Label(this, "StatusLabel", "");

	m_pTabPanel = new PropertySheet(this, "GameTabs");
	m_pTabPanel->SetTabWidth(72);
	m_pTabPanel->AddPage(m_pInternetGames, "#ServerBrowser_InternetTab");
	m_pTabPanel->AddPage(m_pFavorites, "#ServerBrowser_FavoritesTab");
	m_pTabPanel->AddPage(m_pHistory, "#ServerBrowser_HistoryTab");
	m_pTabPanel->AddPage(m_pSpectateGames, "#ServerBrowser_SpectateTab");
	m_pTabPanel->AddPage(m_pLanGames, "#ServerBrowser_LanTab");
	m_pTabPanel->AddPage(m_pFriendsGames, "#ServerBrowser_FriendsTab");
	m_pTabPanel->AddActionSignalTarget(this);

	LoadControlSettings("Servers/DialogServerBrowser.res");

	m_pStatusLabel->SetText("");

	const char *gameList = m_pSavedData->GetString("GameList");

	if (!Q_stricmp(gameList, "spectate"))
	{
		m_pTabPanel->SetActivePage(m_pSpectateGames);
	}
	else if (!Q_stricmp(gameList, "favorites"))
	{
		m_pTabPanel->SetActivePage(m_pFavorites);
	}
	else if (!Q_stricmp(gameList, "history"))
	{
		m_pTabPanel->SetActivePage(m_pHistory);
	}
	else if (!Q_stricmp(gameList, "lan"))
	{
		m_pTabPanel->SetActivePage(m_pLanGames);
	}
	else if (!Q_stricmp(gameList, "friends"))
	{
		m_pTabPanel->SetActivePage(m_pFriendsGames);
	}
	else
	{
		m_pTabPanel->SetActivePage(m_pInternetGames);
	}

	ivgui()->AddTickSignal(GetVPanel());
}

CServerBrowserDialog::~CServerBrowserDialog(void)
{
	SaveUserData();

	if (m_pSavedData)
		m_pSavedData->deleteThis();

	delete m_pContextMenu;
}

void CServerBrowserDialog::Initialize(void)
{
	SetTitle("#ServerBrowser_Servers", true);
	SetVisible(false);
}

gameserveritem_t *CServerBrowserDialog::GetServer(unsigned int serverID)
{
	return m_pGameList->GetServer(serverID);
}

void CServerBrowserDialog::Open(void)
{
	BaseClass::Activate();
	m_pTabPanel->RequestFocus();
}

void CServerBrowserDialog::OnTick(void)
{
	BaseClass::OnTick();
	GetAnimationController()->UpdateAnimations(system()->GetCurrentTime());
}

void CServerBrowserDialog::LoadUserData(void)
{
	if (m_pSavedData)
		m_pSavedData->deleteThis();

	m_pSavedData = new KeyValues("ServerBrowser");

	if (!m_pSavedData->LoadFromFile(g_pFullFileSystem, "cfg/ServerBrowser.vdf"))
	{
	}

	KeyValues *filters = m_pSavedData->FindKey("Filters", false);

	if (filters)
	{
		m_pFilterData = filters->MakeCopy();
		m_pSavedData->RemoveSubKey(filters);
	}
	else
		m_pFilterData = new KeyValues("Filters");

	if (m_pHistory)
	{
		m_pHistory->LoadHistoryList();

		if (m_pHistory->IsVisible())
			m_pHistory->StartRefresh();
	}

	if (m_pFavorites)
	{
		m_pFavorites->LoadFavoritesList();

		ReloadFilterSettings();

		if (m_pFavorites->IsVisible())
			m_pFavorites->StartRefresh();
	}

	InvalidateLayout();
	Repaint();
}

void CServerBrowserDialog::SaveUserData(void)
{
	m_pSavedData->Clear();
	m_pSavedData->LoadFromFile(g_pFullFileSystem, "cfg/ServerBrowser.vdf");

	if (m_pGameList == m_pSpectateGames)
	{
		m_pSavedData->SetString("GameList", "spectate");
	}
	else if (m_pGameList == m_pFavorites)
	{
		m_pSavedData->SetString("GameList", "favorites");
	}
	else if (m_pGameList == m_pLanGames)
	{
		m_pSavedData->SetString("GameList", "lan");
	}
	else if (m_pGameList == m_pFriendsGames)
	{
		m_pSavedData->SetString("GameList", "friends");
	}
	else if (m_pGameList == m_pHistory)
	{
		m_pSavedData->SetString("GameList", "history");
	}
	else
	{
		m_pSavedData->SetString("GameList", "internet");
	}

	m_pSavedData->RemoveSubKey(m_pSavedData->FindKey("Filters"));
	m_pSavedData->AddSubKey(m_pFilterData->MakeCopy());
	m_pSavedData->SaveToFile(g_pFullFileSystem, "cfg/ServerBrowser.vdf");

	SaveUserConfig();
}

void CServerBrowserDialog::RefreshCurrentPage(void)
{
	if (m_pGameList)
	{
		m_pGameList->StartRefresh();
	}
}

void CServerBrowserDialog::UpdateStatusText(const char *fmt, ...)
{
	if (!m_pStatusLabel)
		return;

	if (fmt && strlen(fmt) > 0)
	{
		char str[1024];
		va_list argptr;
		va_start(argptr, fmt);
		_vsnprintf(str, sizeof(str), fmt, argptr);
		va_end(argptr);

		m_pStatusLabel->SetText(str);
	}
	else
	{
		m_pStatusLabel->SetText("");
	}
}

void CServerBrowserDialog::UpdateStatusText(wchar_t *unicode)
{
	if (!m_pStatusLabel)
		return;

	if (unicode && wcslen(unicode) > 0)
	{
		m_pStatusLabel->SetText(unicode);
	}
	else
	{
		m_pStatusLabel->SetText("");
	}
}

void CServerBrowserDialog::OnGameListChanged(void)
{
	m_pGameList = dynamic_cast<IGameList *>(m_pTabPanel->GetActivePage());

	UpdateStatusText("");
	InvalidateLayout();
	Repaint();
}

CServerBrowserDialog *CServerBrowserDialog::GetInstance(void)
{
	return s_InternetDlg;
}

void CServerBrowserDialog::AddServerToFavorites(gameserveritem_t &server)
{
	if (SteamMatchmaking())
	{
		SteamMatchmaking()->AddFavoriteGame(server.m_nAppID, server.m_NetAdr.GetIP(), server.m_NetAdr.GetConnectionPort(),	server.m_NetAdr.GetQueryPort(), k_unFavoriteFlagFavorite, time(NULL));
	}
}

CServerContextMenu *CServerBrowserDialog::GetContextMenu(vgui::Panel *pPanel)
{
	if (m_pContextMenu)
		delete m_pContextMenu;

	m_pContextMenu = new CServerContextMenu(this);
	m_pContextMenu->SetAutoDelete(false);

	if (!pPanel)
		m_pContextMenu->SetParent(this);
	else
		m_pContextMenu->SetParent(pPanel);

	m_pContextMenu->SetVisible(false);
	return m_pContextMenu;
}

CDialogGameInfo *CServerBrowserDialog::JoinGame(IGameList *gameList, unsigned int serverIndex)
{
	CDialogGameInfo *gameDialog = OpenGameInfoDialog(gameList, serverIndex);

	gameDialog->Connect();
	return gameDialog;
}

CDialogGameInfo *CServerBrowserDialog::JoinGame(int serverIP, int serverPort)
{
	CDialogGameInfo *gameDialog = OpenGameInfoDialog(serverIP, serverPort, serverPort);

	gameDialog->Connect();
	return gameDialog;
}

CDialogGameInfo *CServerBrowserDialog::OpenGameInfoDialog(IGameList *gameList, unsigned int serverIndex)
{
	gameserveritem_t *pServer = gameList->GetServer(serverIndex);

	if (!pServer)
		return NULL;

	CDialogGameInfo *gameDialog = new CDialogGameInfo(NULL, pServer->m_NetAdr.GetIP(), pServer->m_NetAdr.GetQueryPort(), pServer->m_NetAdr.GetConnectionPort());
	gameDialog->SetParent(GetVParent());
	gameDialog->AddActionSignalTarget(this);
	gameDialog->Run(pServer->GetName());

	int i = m_GameInfoDialogs.AddToTail();
	m_GameInfoDialogs[i] = gameDialog;
	return gameDialog;
}

CDialogGameInfo *CServerBrowserDialog::OpenGameInfoDialog(int serverIP, uint16 connPort, uint16 queryPort)
{
	CDialogGameInfo *gameDialog = new CDialogGameInfo(NULL, serverIP, queryPort, connPort);
	gameDialog->AddActionSignalTarget(this);
	gameDialog->SetParent(GetVParent());
	gameDialog->Run("");

	int i = m_GameInfoDialogs.AddToTail();
	m_GameInfoDialogs[i] = gameDialog;
	return gameDialog;
}

void CServerBrowserDialog::CloseAllGameInfoDialogs(void)
{
	for (int i = 0; i < m_GameInfoDialogs.Count(); i++)
	{
		vgui::Panel *dlg = m_GameInfoDialogs[i];

		if (dlg)
		{
			vgui::ivgui()->PostMessage(dlg->GetVPanel(), new KeyValues("Close"), NULL);
		}
	}
}

CDialogGameInfo *CServerBrowserDialog::GetDialogGameInfoForFriend(uint64 ulSteamIDFriend)
{
	FOR_EACH_VEC(m_GameInfoDialogs, i)
	{
		CDialogGameInfo *pDlg = m_GameInfoDialogs[i];

		if (pDlg && pDlg->GetAssociatedFriend() == ulSteamIDFriend)
		{
			return pDlg;
		}
	}

	return NULL;
}

KeyValues *CServerBrowserDialog::GetFilterSaveData(const char *filterSet)
{
	return m_pFilterData->FindKey(filterSet, true);
}

const char *CServerBrowserDialog::GetActiveModName(void)
{
	return m_szModDir[0] ? m_szModDir : NULL;
}

const char *CServerBrowserDialog::GetActiveGameName(void)
{
	return m_szGameName[0] ? m_szGameName : NULL;
}

int CServerBrowserDialog::GetActiveAppID(void)
{
	return m_iLimitAppID;
}

void CServerBrowserDialog::OnActiveGameName(KeyValues *pKV)
{
	Q_strncpy(m_szModDir, pKV->GetString("name"), sizeof(m_szModDir));
	Q_strncpy(m_szGameName, pKV->GetString("game"), sizeof(m_szGameName));

	ReloadFilterSettings();
}

void CServerBrowserDialog::ReloadFilterSettings(void)
{
	m_pInternetGames->LoadFilterSettings();
	m_pSpectateGames->LoadFilterSettings();
	m_pFavorites->LoadFilterSettings();
	m_pLanGames->LoadFilterSettings();
	m_pFriendsGames->LoadFilterSettings();
	m_pHistory->LoadFilterSettings();
}

void CServerBrowserDialog::OnConnectToGame(KeyValues *pMessageValues)
{
	int ip = pMessageValues->GetInt("ip");
	int connectionPort = pMessageValues->GetInt("connectionport");
	int queryPort = pMessageValues->GetInt("queryport");

	if (!ip || !queryPort)
		return;

	memset(&m_CurrentConnection, 0, sizeof(gameserveritem_t));
	m_CurrentConnection.m_NetAdr.SetIP(ip);
	m_CurrentConnection.m_NetAdr.SetQueryPort(queryPort);
	m_CurrentConnection.m_NetAdr.SetConnectionPort((unsigned short)connectionPort);

	if (m_pHistory && SteamMatchmaking())
	{
		SteamMatchmaking()->AddFavoriteGame(0, m_CurrentConnection.m_NetAdr.GetIPAndQueryPort().addr_htonl(), connectionPort, queryPort, k_unFavoriteFlagHistory, time(NULL));
		m_pHistory->SetRefreshOnReload();
	}

	for (int i = 0; i < m_GameInfoDialogs.Count(); i++)
	{
		vgui::Panel *dlg = m_GameInfoDialogs[i];

		if (dlg)
		{
			KeyValues *kv = new KeyValues("ConnectedToGame", "ip", ip, "connectionport", connectionPort);
			kv->SetInt("queryport", queryPort);
			vgui::ivgui()->PostMessage(dlg->GetVPanel(), kv, NULL);
		}
	}

	m_pFavorites->OnConnectToGame();
	m_bCurrentlyConnected = true;
}

void CServerBrowserDialog::OnDisconnectFromGame(void)
{
	m_bCurrentlyConnected = false;
	memset(&m_CurrentConnection, 0, sizeof(gameserveritem_t));

	m_pFavorites->OnDisconnectFromGame();
}

void CServerBrowserDialog::ActivateBuildMode(void)
{
	EditablePanel *panel = dynamic_cast<EditablePanel *>(m_pTabPanel->GetActivePage());

	if (!panel)
		return;

	panel->ActivateBuildMode();
}

bool CServerBrowserDialog::GetDefaultScreenPosition(int &x, int &y, int &wide, int &tall)
{
	int wx, wy, ww, wt;
	surface()->GetWorkspaceBounds(wx, wy, ww, wt);

	x = wx + (int)(ww * 0.05);
	y = wy + (int)(wt * 0.4);
	wide = (int)(ww * 0.5);
	tall = (int)(wt * 0.55);
	return true;
}