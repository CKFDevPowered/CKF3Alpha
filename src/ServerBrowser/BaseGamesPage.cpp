#include <time.h>

#include "BaseGamesPage.h"
#include "ServerListCompare.h"
#include "ServerBrowserDialog.h"
#include "ServerBrowser.h"
#include "steam_api.h"
#include "configs.h"

#include <vgui/ILocalize.h>
#include <vgui/IScheme.h>
#include <vgui/IVGui.h>
#include <vgui/KeyCode.h>
#include <KeyValues.h>

#include <vgui_controls/CheckButton.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/ImageList.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/ListPanel.h>

#include <ICommandLine.h>

#define FILTER_ALLSERVERS 0
#define FILTER_SECURESERVERSONLY 1
#define FILTER_INSECURESERVERSONLY 2

#define UNIVERSE_OFFICIAL 0
#define UNIVERSE_CUSTOMGAMES 1

using namespace vgui;

CGameListPanel::CGameListPanel(CBaseGamesPage *pOuter, const char *pName) : BaseClass(pOuter, pName)
{
	m_pOuter = pOuter;
}

void CGameListPanel::OnKeyCodeTyped(vgui::KeyCode code)
{
	if (code == KEY_ENTER && m_pOuter->OnGameListEnterPressed())
		return;

	BaseClass::OnKeyCodeTyped(code);
}

CBaseGamesPage::CBaseGamesPage(vgui::Panel *parent, const char *name, EMatchMakingType eType, const char *pCustomResFilename) : PropertyPage(parent, name), m_pCustomResFilename(pCustomResFilename), m_CallbackFavoritesMsg(this, &CBaseGamesPage::OnFavoritesMsg)
{
	SetSize(624, 278);

	m_szGameFilter[0] = 0;
	m_szMapFilter[0] = 0;
	m_iPingFilter = 0;
	m_iServerRefreshCount = 0;
	m_bFilterNoFullServers = false;
	m_bFilterNoEmptyServers = false;
	m_bFilterNoPasswordedServers = false;
	m_iLimitToAppID = ServerBrowserDialog().GetActiveAppID();
	m_hFont = NULL;
	m_hServerRequest = NULL;
	m_eMatchMakingType = eType;

	SetDefLessFunc(m_mapServers);
	SetDefLessFunc(m_mapServerIP);

	Q_UnicodeToUTF8(g_pVGuiLocalize->Find("ServerBrowser_All"), m_szComboAllText, sizeof(m_szComboAllText));

	m_pConnect = new Button(this, "ConnectButton", "#ServerBrowser_Connect");
	m_pConnect->SetEnabled(false);
	m_pRefreshAll = new Button(this, "RefreshButton", "#ServerBrowser_Refresh");
	m_pRefreshQuick = new Button(this, "RefreshQuickButton", "#ServerBrowser_RefreshQuick");
	m_pAddServer = new Button(this, "AddServerButton", "#ServerBrowser_AddServer");
	m_pAddCurrentServer = new Button(this, "AddCurrentServerButton", "#ServerBrowser_AddCurrentServer");
	m_pGameList = new CGameListPanel(this, "gamelist");
	m_pGameList->SetAllowUserModificationOfColumns(true);

	m_pAddToFavoritesButton = new vgui::Button(this, "AddToFavoritesButton", "");
	m_pAddToFavoritesButton->SetEnabled(false);
	m_pAddToFavoritesButton->SetVisible(false);

	m_pGameList->AddColumnHeader(0, "Password", "#ServerBrowser_Password", 16, ListPanel::COLUMN_FIXEDSIZE | ListPanel::COLUMN_IMAGE);
	m_pGameList->AddColumnHeader(1, "Bots", "#ServerBrowser_Bots", 16, ListPanel::COLUMN_FIXEDSIZE | ListPanel::COLUMN_HIDDEN);
	m_pGameList->AddColumnHeader(2, "Secure", "#ServerBrowser_Secure", 16, ListPanel::COLUMN_FIXEDSIZE | ListPanel::COLUMN_IMAGE);
	m_pGameList->AddColumnHeader(3, "Name", "#ServerBrowser_Servers", 50, ListPanel::COLUMN_RESIZEWITHWINDOW | ListPanel::COLUMN_UNHIDABLE);
	m_pGameList->AddColumnHeader(4, "IPAddr", "#ServerBrowser_IPAddress", 64, ListPanel::COLUMN_HIDDEN);
	m_pGameList->AddColumnHeader(5, "GameDesc", "#ServerBrowser_Game", 112, 112, 300, 0);
	m_pGameList->AddColumnHeader(6, "Players", "#ServerBrowser_Players", 55, ListPanel::COLUMN_FIXEDSIZE);
	m_pGameList->AddColumnHeader(7, "Map", "#ServerBrowser_Map", 90, 90, 300, 0);
	m_pGameList->AddColumnHeader(8, "Ping", "#ServerBrowser_Latency", 55, ListPanel::COLUMN_FIXEDSIZE);

	m_pGameList->SetColumnHeaderTooltip(0, "#ServerBrowser_PasswordColumn_Tooltip");
	m_pGameList->SetColumnHeaderTooltip(1, "#ServerBrowser_BotColumn_Tooltip");
	m_pGameList->SetColumnHeaderTooltip(2, "#ServerBrowser_SecureColumn_Tooltip");

	m_pGameList->SetSortFunc(0, PasswordCompare);
	m_pGameList->SetSortFunc(1, BotsCompare);
	m_pGameList->SetSortFunc(2, SecureCompare);
	m_pGameList->SetSortFunc(3, ServerNameCompare);
	m_pGameList->SetSortFunc(4, IPAddressCompare);
	m_pGameList->SetSortFunc(5, GameCompare);
	m_pGameList->SetSortFunc(6, PlayersCompare);
	m_pGameList->SetSortFunc(7, MapCompare);
	m_pGameList->SetSortFunc(8, PingCompare);

	m_pGameList->SetSortColumn(8);

	ivgui()->AddTickSignal(GetVPanel());

	CreateFilters();
	LoadFilterSettings();

	m_bAutoSelectFirstItemInGameList = false;
}

CBaseGamesPage::~CBaseGamesPage(void)
{
}

int CBaseGamesPage::GetInvalidServerListID(void)
{
	return m_pGameList->InvalidItemID();
}

void CBaseGamesPage::PerformLayout(void)
{
	BaseClass::PerformLayout();

	if (m_pGameList->GetSelectedItemsCount() < 1)
		m_pConnect->SetEnabled(false);
	else
		m_pConnect->SetEnabled(true);

	if (SupportsItem(IGameList::GETNEWLIST))
	{
		m_pFilterString->SetWide(115);
		m_pRefreshQuick->SetVisible(true);
		m_pRefreshAll->SetText("#ServerBrowser_RefreshAll");
	}
	else
	{
		m_pRefreshQuick->SetVisible(false);
		m_pRefreshAll->SetText("#ServerBrowser_Refresh");
	}

	if (SupportsItem(IGameList::ADDSERVER))
	{
		m_pFilterString->SetWide(115);
		m_pAddServer->SetVisible(true);
	}
	else
	{
		m_pAddServer->SetVisible(false);
	}

	if (SupportsItem(IGameList::ADDCURRENTSERVER))
		m_pAddCurrentServer->SetVisible(true);
	else
		m_pAddCurrentServer->SetVisible(false);

	if (IsRefreshing())
		m_pRefreshAll->SetText("#ServerBrowser_StopRefreshingList");

	if (m_pGameList->GetItemCount() > 0)
		m_pRefreshQuick->SetEnabled(true);
	else
		m_pRefreshQuick->SetEnabled(false);

	if (!SteamMatchmakingServers() || !SteamMatchmaking())
	{
		m_pAddCurrentServer->SetVisible(false);
		m_pRefreshQuick->SetEnabled(false);
		m_pAddServer->SetEnabled(false);
		m_pConnect->SetEnabled(false);
		m_pRefreshAll->SetEnabled(false);
		m_pAddToFavoritesButton->SetEnabled(false);
		m_pGameList->SetEmptyListText("#ServerBrowser_SteamRunning");
	}

	Repaint();
}

void CBaseGamesPage::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	ImageList *imageList = new ImageList(false);
	imageList->AddImage(scheme()->GetImage("servers/icon_password", false));
	imageList->AddImage(scheme()->GetImage("servers/icon_bots", false));
	imageList->AddImage(scheme()->GetImage("servers/icon_robotron", false));
	imageList->AddImage(scheme()->GetImage("servers/icon_secure_deny", false));

	int passwordColumnImage = imageList->AddImage(scheme()->GetImage("servers/icon_password_column", false));
	int botColumnImage = imageList->AddImage(scheme()->GetImage("servers/icon_bots_column", false));
	int secureColumnImage = imageList->AddImage(scheme()->GetImage("servers/icon_robotron_column", false));

	m_pGameList->SetImageList(imageList, true);
	m_hFont = pScheme->GetFont("ListSmall", IsProportional());

	if (!m_hFont)
		m_hFont = pScheme->GetFont("DefaultSmall", IsProportional());

	m_pGameList->SetFont(m_hFont);
	m_pGameList->SetColumnHeaderImage(0, passwordColumnImage);
	m_pGameList->SetColumnHeaderImage(1, botColumnImage);
	m_pGameList->SetColumnHeaderImage(2, secureColumnImage);

	OnButtonToggled(m_pFilter, false);
}

gameserveritem_t *CBaseGamesPage::GetServer(unsigned int serverID)
{
	if (!SteamMatchmakingServers())
		return NULL;

	if (serverID >= 0)
	{
		return SteamMatchmakingServers()->GetServerDetails(m_hServerRequest, serverID);
	}
	else
	{
		Assert(!"Unable to return a useful entry");
		return NULL;
	}
}

void CBaseGamesPage::CreateFilters(void)
{
	m_bFiltersVisible = false;
	m_pFilter = new ToggleButton(this, "Filter", "#ServerBrowser_Filter");
	m_pFilterString = new Label(this, "FilterString", "");
	m_pGameFilter = new TextEntry(this, "GameFilter");
	m_pLocationFilter = new ComboBox(this, "LocationFilter", 6, false);
	m_pMapFilter = new TextEntry(this, "MapFilter");
	m_pPingFilter = new ComboBox(this, "PingFilter", 6, false);
	m_pPingFilter->AddItem("#ServerBrowser_All", NULL);
	m_pPingFilter->AddItem("#ServerBrowser_LessThan50", NULL);
	m_pPingFilter->AddItem("#ServerBrowser_LessThan100", NULL);
	m_pPingFilter->AddItem("#ServerBrowser_LessThan150", NULL);
	m_pPingFilter->AddItem("#ServerBrowser_LessThan250", NULL);
	m_pPingFilter->AddItem("#ServerBrowser_LessThan350", NULL);
	m_pPingFilter->AddItem("#ServerBrowser_LessThan600", NULL);

	m_pSecureFilter = new ComboBox(this, "SecureFilter", 3, false);
	m_pSecureFilter->AddItem("#ServerBrowser_All", NULL);
	m_pSecureFilter->AddItem("#ServerBrowser_SecureOnly", NULL);
	m_pSecureFilter->AddItem("#ServerBrowser_InsecureOnly", NULL);

	m_pNoEmptyServersFilterCheck = new CheckButton(this, "ServerEmptyFilterCheck", "");
	m_pNoFullServersFilterCheck = new CheckButton(this, "ServerFullFilterCheck", "");
	m_pNoPasswordFilterCheck = new CheckButton(this, "NoPasswordFilterCheck", "");
}

void CBaseGamesPage::LoadFilterSettings(void)
{
	KeyValues *filter = ServerBrowserDialog().GetFilterSaveData(GetName());

	Q_strncpy(m_szGameFilter, filter->GetString("game"), sizeof(m_szGameFilter));
	Q_strncpy(m_szMapFilter, filter->GetString("map"), sizeof(m_szMapFilter));

	m_iPingFilter = filter->GetInt("ping");
	m_bFilterNoFullServers = filter->GetInt("NoFull");
	m_bFilterNoEmptyServers = filter->GetInt("NoEmpty");
	m_bFilterNoPasswordedServers = filter->GetInt("NoPassword");

	UpdateGameFilter();

	m_pGameFilter->SetText(m_szGameFilter);
	m_pMapFilter->SetText(m_szMapFilter);
	m_pLocationFilter->ActivateItem(filter->GetInt("location"));

	if (m_iPingFilter)
	{
		char buf[32];
		Q_snprintf(buf, sizeof(buf), "< %d", m_iPingFilter);
		m_pPingFilter->SetText(buf);
	}

	m_pNoFullServersFilterCheck->SetSelected(m_bFilterNoFullServers);
	m_pNoEmptyServersFilterCheck->SetSelected(m_bFilterNoEmptyServers);
	m_pNoPasswordFilterCheck->SetSelected(m_bFilterNoPasswordedServers);

	LoadFilter(filter);
	UpdateFilterSettings();
}

void CBaseGamesPage::UpdateGameFilter(void)
{
	if (m_pLocationFilter->GetItemCount() > 0)
		m_pLocationFilter->SetEnabled(true);

	m_pSecureFilter->SetVisible(false);
}

void CBaseGamesPage::ServerResponded(gameserveritem_t &server)
{
	int nIndex = -1;

	while (m_mapServers.Find(nIndex) != m_mapServers.InvalidIndex())
		nIndex--;

	ServerResponded(nIndex, &server);
}

void CBaseGamesPage::ServerResponded(HServerListRequest hRequest, int iServer)
{
	gameserveritem_t *pServerItem = SteamMatchmakingServers()->GetServerDetails(hRequest, iServer);

	if (!pServerItem)
	{
		Assert(!"Missing server response");
		return;
	}

	ServerResponded(iServer, pServerItem);
}

void CBaseGamesPage::ServerResponded(int iServer, gameserveritem_t *pServerItem)
{
	int iServerMap = m_mapServers.Find(iServer);

	if (iServerMap == m_mapServers.InvalidIndex())
	{
		netadr_t netAdr(pServerItem->m_NetAdr.GetIP(), pServerItem->m_NetAdr.GetQueryPort());
		int iServerIP = m_mapServerIP.Find(netAdr);

		if (iServerIP != m_mapServerIP.InvalidIndex())
		{
			int iServerMap = m_mapServers.Find(m_mapServerIP[iServerIP]);

			if (iServerMap != m_mapServers.InvalidIndex())
			{
				serverdisplay_t &server = m_mapServers[iServerMap];

				if (m_pGameList->IsValidItemID(server.m_iListID))
					m_pGameList->RemoveItem(server.m_iListID);

				m_mapServers.RemoveAt(iServerMap);
			}

			m_mapServerIP.RemoveAt(iServerIP);
		}

		serverdisplay_t serverFind;
		serverFind.m_iListID = -1;
		serverFind.m_bDoNotRefresh = false;
		iServerMap = m_mapServers.Insert(iServer, serverFind);
		m_mapServerIP.Insert(netAdr, iServer);
	}

	serverdisplay_t *pServer = &m_mapServers[iServerMap];
	pServer->m_iServerID = iServer;
	Assert(pServerItem->m_NetAdr.GetIP() != 0);

	bool removeItem = false;

	if (!CheckPrimaryFilters(*pServerItem))
	{
		pServer->m_bDoNotRefresh = true;

		removeItem = true;
	}
	else if (!CheckSecondaryFilters(*pServerItem))
	{
		removeItem = true;
	}

	if (removeItem)
	{
		if (m_pGameList->IsValidItemID(pServer->m_iListID))
		{
			m_pGameList->RemoveItem(pServer->m_iListID);
			pServer->m_iListID = GetInvalidServerListID();
		}

		return;
	}

	KeyValues *kv;

	if (m_pGameList->IsValidItemID(pServer->m_iListID))
	{
		kv = m_pGameList->GetItem(pServer->m_iListID);
		m_pGameList->SetUserData(pServer->m_iListID, pServer->m_iServerID);
	}
	else
	{
		kv = new KeyValues("Server");
	}

	kv->SetString("name", pServerItem->GetName());
	kv->SetString("map", pServerItem->m_szMap);
	kv->SetString("GameDir", pServerItem->m_szGameDir);
	kv->SetString("GameDesc", pServerItem->m_szGameDescription);
	kv->SetInt("password", pServerItem->m_bPassword ? 1 : 0);

	if (pServerItem->m_nBotPlayers > 0)
		kv->SetInt("bots", pServerItem->m_nBotPlayers);
	else
		kv->SetString("bots", "");

	if (pServerItem->m_bSecure)
	{
		kv->SetInt("secure", ServerBrowser().IsVACBannedFromGame(pServerItem->m_nAppID) ? 4 : 3);
	}
	else
	{
		kv->SetInt("secure", 0);
	}

	kv->SetString("IPAddr", pServerItem->m_NetAdr.GetConnectionAddressString());

	int nAdjustedForBotsPlayers = max(0, pServerItem->m_nPlayers - pServerItem->m_nBotPlayers);
	int nAdjustedForBotsMaxPlayers = max(0, pServerItem->m_nMaxPlayers - pServerItem->m_nBotPlayers);

	char buf[32];
	Q_snprintf(buf, sizeof(buf), "%d / %d", nAdjustedForBotsPlayers, nAdjustedForBotsMaxPlayers);
	kv->SetString("Players", buf);

	kv->SetInt("Ping", pServerItem->m_nPing);
	kv->SetString("Tags", pServerItem->m_szGameTags);

	if (pServerItem->m_ulTimeLastPlayed)
	{
		struct tm *now;
		now = localtime((time_t *)&pServerItem->m_ulTimeLastPlayed);

		if (now)
		{
			char buf[64];
			strftime(buf, sizeof(buf), "%a %d %b %I:%M%p", now);
			Q_strlower(buf + strlen(buf) - 4);
			kv->SetString("LastPlayed", buf);
		}
	}

	if (pServer->m_bDoNotRefresh)
	{
		kv->SetString("Ping", "");
		kv->SetWString("GameDesc", g_pVGuiLocalize->Find("#ServerBrowser_NotResponding"));
		kv->SetString("Players", "");
		kv->SetString("map", "");
	}

	if (!m_pGameList->IsValidItemID(pServer->m_iListID))
	{
		pServer->m_iListID = m_pGameList->AddItem(kv, pServer->m_iServerID, false, false);

		if (m_bAutoSelectFirstItemInGameList && m_pGameList->GetItemCount() == 1)
			m_pGameList->AddSelectedItem(pServer->m_iListID);

		kv->deleteThis();
	}
	else
	{
		m_pGameList->ApplyItemChanges(pServer->m_iListID);
		m_pGameList->SetItemVisible(pServer->m_iListID, true);
	}

	UpdateStatus();
	m_iServerRefreshCount++;
}

void CBaseGamesPage::OnButtonToggled(Panel *panel, int state)
{
	if (panel == m_pFilter)
	{
		int wide, tall;
		GetSize(wide, tall);
		SetSize(624, 278);

		if (m_pCustomResFilename)
		{
			m_bFiltersVisible = false;
		}
		else
		{
			if (m_pFilter->IsSelected())
				m_bFiltersVisible = true;
			else
				m_bFiltersVisible = false;
		}

		UpdateDerivedLayouts();

		m_pFilter->SetSelected(m_bFiltersVisible);

		UpdateGameFilter();

		if (m_hFont)
		{
			SETUP_PANEL(m_pGameList);
			m_pGameList->SetFont(m_hFont);
		}

		SetSize(wide, tall);
		InvalidateLayout();
	}
	else if (panel == m_pNoFullServersFilterCheck || panel == m_pNoEmptyServersFilterCheck || panel == m_pNoPasswordFilterCheck)
	{
		OnTextChanged(panel, "");
	}
}

void CBaseGamesPage::UpdateDerivedLayouts(void)
{
	char rgchControlSettings[MAX_PATH];

	if (m_pCustomResFilename)
	{
		Q_snprintf(rgchControlSettings, sizeof(rgchControlSettings), "%s", m_pCustomResFilename);
	}
	else
	{
		if (m_pFilter->IsSelected())
		{
			Q_snprintf(rgchControlSettings, sizeof(rgchControlSettings), "servers/%sPage_Filters.res", "InternetGames");
		}
		else
		{
			Q_snprintf(rgchControlSettings, sizeof(rgchControlSettings), "servers/%sPage.res", "InternetGames");
		}
	}

	LoadControlSettings(rgchControlSettings);
}

void CBaseGamesPage::OnTextChanged(Panel *panel, const char *text)
{
	if (!Q_stricmp(text, m_szComboAllText))
	{
		ComboBox *box = dynamic_cast<ComboBox *>(panel);

		if (box)
		{
			box->SetText("");
			text = "";
		}
	}

	UpdateFilterSettings();
	ApplyFilters();

	if (m_bFiltersVisible && (panel == m_pLocationFilter))
	{
		StopRefresh(); 
		GetNewServerList(); 
	}
}

void CBaseGamesPage::ApplyFilters(void)
{
	ApplyGameFilters();
}

void CBaseGamesPage::ApplyGameFilters(void)
{
	if (!SteamMatchmakingServers())
		return;

	FOR_EACH_MAP_FAST(m_mapServers, i)
	{
		serverdisplay_t &server = m_mapServers[i];
		gameserveritem_t *pServer = SteamMatchmakingServers()->GetServerDetails(m_hServerRequest, server.m_iServerID);

		if (!pServer)
			continue;

		if (!CheckPrimaryFilters(*pServer) || !CheckSecondaryFilters(*pServer))
		{
			server.m_bDoNotRefresh = true;

			if (m_pGameList->IsValidItemID(server.m_iListID))
			{
				m_pGameList->SetItemVisible(server.m_iListID, false);
			}
		}
		else if (BShowServer(server))
		{
			server.m_bDoNotRefresh = false;
			gameserveritem_t *pServer = SteamMatchmakingServers()->GetServerDetails(m_hServerRequest, server.m_iServerID);

			if (!m_pGameList->IsValidItemID(server.m_iListID))
			{
				KeyValues *kv = new KeyValues("Server");
				kv->SetString("name", pServer->GetName());
				kv->SetString("map", pServer->m_szMap);
				kv->SetString("GameDir", pServer->m_szGameDir);

				if (pServer->m_szGameDescription[0])
				{
					kv->SetString("GameDesc", pServer->m_szGameDescription);
				}
				else
				{
					kv->SetWString("GameDesc", g_pVGuiLocalize->Find("#ServerBrowser_PendingPing"));
				}

				int nAdjustedForBotsPlayers = max(0, pServer->m_nPlayers - pServer->m_nBotPlayers);
				int nAdjustedForBotsMaxPlayers = max(0, pServer->m_nMaxPlayers - pServer->m_nBotPlayers);

				char buf[256];
				Q_snprintf(buf, sizeof(buf), "%d / %d", nAdjustedForBotsPlayers, nAdjustedForBotsMaxPlayers);
				kv->SetString("Players", buf);
				kv->SetInt("Ping", pServer->m_nPing);
				kv->SetInt("password", pServer->m_bPassword ? 1 : 0);

				if (pServer->m_nBotPlayers > 0)
					kv->SetInt("bots", pServer->m_nBotPlayers);
				else
					kv->SetString("bots", "");

				server.m_iListID = m_pGameList->AddItem(kv, server.m_iServerID, false, false);
				kv->deleteThis();
			}

			m_pGameList->SetItemVisible(server.m_iListID, true);
		}
	}

	UpdateStatus();
	m_pGameList->SortList();
	InvalidateLayout();
	Repaint();
}

void CBaseGamesPage::UpdateStatus(void)
{
	if (m_pGameList->GetItemCount() > 1)
	{
		wchar_t header[256];
		wchar_t count[128];

		_itow(m_pGameList->GetItemCount(), count, 10);
		g_pVGuiLocalize->ConstructString(header, sizeof(header), g_pVGuiLocalize->Find("#ServerBrowser_ServersCount"), 1, count);
		m_pGameList->SetColumnHeaderText(2, header);
	}
	else
	{
		m_pGameList->SetColumnHeaderText(2, g_pVGuiLocalize->Find("#ServerBrowser_Servers"));
	}
}

void CBaseGamesPage::UpdateFilterSettings(void)
{
	m_pGameFilter->GetText(m_szGameFilter, sizeof(m_szGameFilter) - 1);
	m_pMapFilter->GetText(m_szMapFilter, sizeof(m_szMapFilter) - 1);

	char buf[256];
	m_pPingFilter->GetText(buf, sizeof(buf));

	if (buf[0])
	{
		m_iPingFilter = atoi(buf + 2);
	}
	else
	{
		m_iPingFilter = 0;
	}

	m_bFilterNoFullServers = m_pNoFullServersFilterCheck->IsSelected();
	m_bFilterNoEmptyServers = m_pNoEmptyServersFilterCheck->IsSelected();
	m_bFilterNoPasswordedServers = m_pNoPasswordFilterCheck->IsSelected();
	m_iSecureFilter = m_pSecureFilter->GetActiveItem();

	m_vecServerFilters.RemoveAll();
	m_vecServerFilters.AddToTail(MatchMakingKeyValuePair_t("gamedir", gConfigs.szGameDir));

	if (m_bFilterNoEmptyServers)
		m_vecServerFilters.AddToTail(MatchMakingKeyValuePair_t("empty", "1"));

	if (m_bFilterNoFullServers)
		m_vecServerFilters.AddToTail(MatchMakingKeyValuePair_t("full", "1"));

	if (m_iSecureFilter == FILTER_SECURESERVERSONLY)
		m_vecServerFilters.AddToTail(MatchMakingKeyValuePair_t("secure", "1"));

	int regCode = GetRegionCodeToFilter();

	if (regCode > 0)
	{
		char szRegCode[32];
		Q_snprintf(szRegCode, sizeof(szRegCode), "%i", regCode);
		m_vecServerFilters.AddToTail(MatchMakingKeyValuePair_t("region", szRegCode));		
	}

	KeyValues *filter = ServerBrowserDialog().GetFilterSaveData(GetName());

	filter->SetString("map", m_szMapFilter);
	filter->SetInt("ping", m_iPingFilter);

	if (m_pLocationFilter->GetItemCount() > 1)
		filter->SetInt("location", m_pLocationFilter->GetActiveItem());

	filter->SetInt("NoFull", m_bFilterNoFullServers);
	filter->SetInt("NoEmpty", m_bFilterNoEmptyServers);
	filter->SetInt("NoPassword", m_bFilterNoPasswordedServers);
	filter->SetInt("Secure", m_iSecureFilter);

	filter->SetString("gametype", "notags");
	m_vecServerFilters.AddToTail(MatchMakingKeyValuePair_t("gametype", "notags"));

	SaveFilter(filter);
	RecalculateFilterString();
}

void CBaseGamesPage::OnSaveFilter(KeyValues *filter)
{
}

void CBaseGamesPage::OnLoadFilter(KeyValues *filter)
{
}

void CBaseGamesPage::LoadFilter(KeyValues *filter)
{
}

void CBaseGamesPage::SaveFilter(KeyValues *filter)
{
	filter->SetString("game", m_szGameFilter);
	filter->SetString("map", m_szMapFilter);
	filter->SetInt("ping", m_iPingFilter);

	if (m_pLocationFilter->GetItemCount() > 1)
		filter->SetInt("location", m_pLocationFilter->GetActiveItem());

	filter->SetInt("NoFull", m_bFilterNoFullServers);
	filter->SetInt("NoEmpty", m_bFilterNoEmptyServers);
	filter->SetInt("NoPassword", m_bFilterNoPasswordedServers);

	OnSaveFilter(filter);
}

void CBaseGamesPage::RecalculateFilterString(void)
{
	wchar_t unicode[2048], tempUnicode[128], spacerUnicode[8];
	unicode[0] = 0;

	Q_UTF8ToUnicode("; ", spacerUnicode, sizeof(spacerUnicode));

	if (m_szGameFilter[0])
	{
		Q_UTF8ToUnicode(m_szGameFilter, tempUnicode, sizeof(tempUnicode));
		wcscat(unicode, tempUnicode);
		wcscat(unicode, spacerUnicode);
	}

	if (m_pLocationFilter->GetActiveItem() > 0)
	{
		m_pLocationFilter->GetText(tempUnicode, sizeof(tempUnicode));
		wcscat(unicode, tempUnicode);
		wcscat(unicode, spacerUnicode);
	}

	if (m_iPingFilter)
	{
		char tmpBuf[16];
		_itoa(m_iPingFilter, tmpBuf, 10);

		wcscat(unicode, g_pVGuiLocalize->Find("#ServerBrowser_FilterDescLatency"));
		Q_UTF8ToUnicode(" < ", tempUnicode, sizeof(tempUnicode));
		wcscat(unicode, tempUnicode);
		Q_UTF8ToUnicode(tmpBuf, tempUnicode, sizeof(tempUnicode));
		wcscat(unicode, tempUnicode);	
		wcscat(unicode, spacerUnicode);
	}

	if (m_bFilterNoFullServers)
	{
		wcscat(unicode, g_pVGuiLocalize->Find("#ServerBrowser_FilterDescNotFull"));
		wcscat(unicode, spacerUnicode);
	}

	if (m_bFilterNoEmptyServers)
	{
		wcscat(unicode, g_pVGuiLocalize->Find("#ServerBrowser_FilterDescNotEmpty"));
		wcscat(unicode, spacerUnicode);
	}

	if (m_bFilterNoPasswordedServers)
	{
		wcscat(unicode, g_pVGuiLocalize->Find("#ServerBrowser_FilterDescNoPassword"));
		wcscat(unicode, spacerUnicode);
	}

	if (m_szMapFilter[0])
	{
		Q_UTF8ToUnicode(m_szMapFilter, tempUnicode, sizeof(tempUnicode));
		wcscat(unicode, tempUnicode);
	}

	m_pFilterString->SetText(unicode);
}

bool CBaseGamesPage::CheckPrimaryFilters(gameserveritem_t &server)
{
	if (server.m_nMaxPlayers < 1)
		return false;

	if (Q_strlen(server.m_szServerName) < 0)
		return false;

	if (server.m_bSecure)
		return false;

	int count = Q_strlen(m_szGameFilter);

	if (count && (server.m_szGameDir[0] || server.m_nPing) && Q_strnicmp(server.m_szGameDescription, m_szGameFilter, count))
		return false;

	return true;
}

bool CBaseGamesPage::CheckSecondaryFilters(gameserveritem_t &server)
{
	if (m_bFilterNoEmptyServers && (server.m_nPlayers - server.m_nBotPlayers) < 1)
		return false;

	if (m_bFilterNoFullServers && server.m_nPlayers >= server.m_nMaxPlayers)
		return false;

	if (m_iPingFilter && server.m_nPing > m_iPingFilter)
		return false;

	if (m_bFilterNoPasswordedServers && server.m_bPassword)
		return false;

	if (m_iSecureFilter == FILTER_SECURESERVERSONLY && !server.m_bSecure)
		return false;

	if (m_iSecureFilter == FILTER_INSECURESERVERSONLY && server.m_bSecure)
		return false;

	int count = Q_strlen(m_szMapFilter);

	if (count && Q_strnicmp(server.m_szMap, m_szMapFilter, count))
		return false;

	return true;
}

uint32 CBaseGamesPage::GetServerFilters(MatchMakingKeyValuePair_t **pFilters)
{
	*pFilters = m_vecServerFilters.Base();
	return m_vecServerFilters.Count();
}

void CBaseGamesPage::SetRefreshing(bool state)
{
	if (state)
	{
		ServerBrowserDialog().UpdateStatusText("#ServerBrowser_RefreshingServerList");

		m_pGameList->SetEmptyListText("");
		m_pRefreshAll->SetText("#ServerBrowser_StopRefreshingList");
		m_pRefreshAll->SetCommand("stoprefresh");
		m_pRefreshQuick->SetEnabled(false);
	}
	else
	{
		ServerBrowserDialog().UpdateStatusText("");

		if (SupportsItem(IGameList::GETNEWLIST))
			m_pRefreshAll->SetText("#ServerBrowser_RefreshAll");
		else
			m_pRefreshAll->SetText("#ServerBrowser_Refresh");

		m_pRefreshAll->SetCommand("GetNewList");

		if (m_pGameList->GetItemCount() > 0)
			m_pRefreshQuick->SetEnabled(true);
		else
			m_pRefreshQuick->SetEnabled(false);
	}
}

void CBaseGamesPage::OnCommand(const char *command)
{
	if (!Q_stricmp(command, "Connect"))
	{
		OnBeginConnect();
	}
	else if (!Q_stricmp(command, "stoprefresh"))
	{
		StopRefresh();
	}
	else if (!Q_stricmp(command, "refresh"))
	{
		if (SteamMatchmakingServers())
			SteamMatchmakingServers()->RefreshQuery(m_hServerRequest);

		SetRefreshing(true);
		m_iServerRefreshCount = 0;
	}
	else if (!Q_stricmp(command, "GetNewList"))
	{
		GetNewServerList();
	}
	else
	{
		BaseClass::OnCommand(command);
	}
}

void CBaseGamesPage::OnItemSelected(void)
{
	if (m_pGameList->GetSelectedItemsCount() < 1)
		m_pConnect->SetEnabled(false);
	else
		m_pConnect->SetEnabled(true);
}

void CBaseGamesPage::OnKeyCodePressed(vgui::KeyCode code)
{
	if (code == KEY_F5)
	{
		StartRefresh();
	}
	else
	{
		BaseClass::OnKeyCodePressed(code);
	}
}

bool CBaseGamesPage::OnGameListEnterPressed(void)
{
	return false;
}

int CBaseGamesPage::GetSelectedItemsCount(void)
{
	return m_pGameList->GetSelectedItemsCount();
}

void CBaseGamesPage::OnAddToFavorites(void)
{
	if (!SteamMatchmakingServers())
		return;

	for (int i = 0; i < m_pGameList->GetSelectedItemsCount(); i++)
	{
		int serverID = m_pGameList->GetItemUserData(m_pGameList->GetSelectedItem(i));
		gameserveritem_t *pServer = SteamMatchmakingServers()->GetServerDetails(m_hServerRequest, serverID);

		if (pServer)
		{
			ServerBrowserDialog().AddServerToFavorites(*pServer);
		}
	}
}

void CBaseGamesPage::ServerFailedToRespond(HServerListRequest hRequest, int iServer)
{
	ServerResponded(hRequest, iServer);
}

void CBaseGamesPage::RemoveServer(serverdisplay_t &server)
{
	if (m_pGameList->IsValidItemID(server.m_iListID))
	{
		m_pGameList->SetItemVisible(server.m_iListID, false);
	}

	UpdateStatus();
}

void CBaseGamesPage::OnRefreshServer(int serverID)
{
	if (!SteamMatchmakingServers())
		return;

	for (int i = 0; i < m_pGameList->GetSelectedItemsCount(); i++)
	{
		int serverID = m_pGameList->GetItemUserData(m_pGameList->GetSelectedItem(i));
		SteamMatchmakingServers()->RefreshServer(m_hServerRequest, serverID);
	}

	SetRefreshing(IsRefreshing());
}

void CBaseGamesPage::StartRefresh(void)
{
	if (!SteamMatchmakingServers())
		return;

	ClearServerList();

	MatchMakingKeyValuePair_t *pFilters;
	int nFilters = GetServerFilters(&pFilters);

	switch (m_eMatchMakingType)
	{
		case eFavoritesServer:
		{
			m_hServerRequest = SteamMatchmakingServers()->RequestFavoritesServerList(GetFilterAppID(), &pFilters, nFilters, this);
			break;
		}

		case eHistoryServer:
		{
			m_hServerRequest = SteamMatchmakingServers()->RequestHistoryServerList(GetFilterAppID(), &pFilters, nFilters, this);
			break;
		}

		case eInternetServer:
		{
			m_hServerRequest = SteamMatchmakingServers()->RequestInternetServerList(GetFilterAppID(), &pFilters, nFilters, this);
			break;
		}

		case eSpectatorServer:
		{
			m_hServerRequest = SteamMatchmakingServers()->RequestSpectatorServerList(GetFilterAppID(), &pFilters, nFilters, this);
			break;
		}

		case eFriendsServer:
		{
			m_hServerRequest = SteamMatchmakingServers()->RequestFriendsServerList(GetFilterAppID(), &pFilters, nFilters, this);
			break;
		}

		case eLANServer:
		{
			m_hServerRequest = SteamMatchmakingServers()->RequestLANServerList(GetFilterAppID(), this);
			break;
		}

		default:
		{
			Assert(!"Unknown server type");
			break;
		}
	}

	SetRefreshing(true);

	m_iServerRefreshCount = 0;
}

void CBaseGamesPage::ClearServerList(void)
{ 
	m_hServerRequest = NULL;
	m_mapServers.RemoveAll(); 
	m_mapServerIP.RemoveAll();
	m_pGameList->RemoveAll();
}

void CBaseGamesPage::GetNewServerList(void)
{
	StartRefresh();
}

void CBaseGamesPage::StopRefresh(void)
{
	m_iServerRefreshCount = 0;

	if (SteamMatchmakingServers())
		SteamMatchmakingServers()->CancelQuery(m_hServerRequest);

	RefreshComplete(m_hServerRequest, eServerResponded);
}

bool CBaseGamesPage::IsRefreshing(void)
{
	return false;
}

void CBaseGamesPage::OnPageShow(void)
{
	StartRefresh();
}

void CBaseGamesPage::OnPageHide(void)
{
	StopRefresh();
}

void CBaseGamesPage::OnBeginConnect(void)
{
	if (!m_pGameList->GetSelectedItemsCount())
		return;

	int serverID = m_pGameList->GetItemUserData(m_pGameList->GetSelectedItem(0));

	StopRefresh();
	ServerBrowserDialog().JoinGame(this, serverID);
}

void CBaseGamesPage::OnViewGameInfo(void)
{
	if (!m_pGameList->GetSelectedItemsCount())
		return;

	int serverID = m_pGameList->GetItemUserData(m_pGameList->GetSelectedItem(0));

	StopRefresh();
	ServerBrowserDialog().OpenGameInfoDialog(this, serverID);
}

void CBaseGamesPage::OnFavoritesMsg(FavoritesListChanged_t *pFavListChanged)
{
	switch (m_eMatchMakingType)
	{
		case eInternetServer:
		case eLANServer:
		case eSpectatorServer:
		case eFriendsServer:
		{
			break;
		}

		case eFavoritesServer:
		case eHistoryServer:
		{
			int iIPServer = m_mapServerIP.Find(netadr_t(pFavListChanged->m_nIP, pFavListChanged->m_nQueryPort));

			if (iIPServer == m_mapServerIP.InvalidIndex())
			{
				if (pFavListChanged->m_bAdd)
				{
					if (SteamMatchmakingServers())
						SteamMatchmakingServers()->PingServer(pFavListChanged->m_nIP, pFavListChanged->m_nQueryPort, this);
				}
			}
			else
			{
				if (pFavListChanged->m_bAdd)
				{
					if (m_mapServerIP[iIPServer] > 0)
						ServerResponded(m_hServerRequest, m_mapServerIP[iIPServer]);
				}
				else
				{
					int iServer = m_mapServers.Find(m_mapServerIP[iIPServer]);
					serverdisplay_t &server = m_mapServers[iServer];
					RemoveServer(server);
				}
			}

			break;
		}

		default:
		{
			Assert(!"unknown matchmaking type");
		}
	}
}