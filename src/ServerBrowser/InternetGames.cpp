#include <stdio.h>

#include "InternetGames.h"
#include "ServerContextMenu.h"
#include "ServerListCompare.h"
#include "ServerBrowserDialog.h"
#include "steam_api.h"
#include "OfflineMode.h"

#include <vgui/ILocalize.h>
#include <vgui/IInput.h>
#include <vgui/IScheme.h>
#include <vgui/IVGui.h>
#include <vgui/MouseCode.h>
#include <KeyValues.h>

#include <vgui_controls/Button.h>
#include <vgui_controls/ToggleButton.h>
#include <vgui_controls/Menu.h>
#include <vgui_controls/ComboBox.h>

using namespace vgui;

const int NUMBER_OF_RETRIES = 3;
const float MASTER_LIST_TIMEOUT = 3.0f;
const float MINIMUM_SORT_TIME = 1.5f;
const int MAXIMUM_SERVERS = 10000;

CInternetGames::CInternetGames(vgui::Panel *parent, const char *panelName, EMatchMakingType eType) : CBaseGamesPage(parent, panelName, eType)
{
	m_fLastSort = 0.0f;
	m_bDirty = false;
	m_bRequireUpdate = true;
	m_bOfflineMode = !IsSteamGameServerBrowsingEnabled();

	m_bAnyServersRetrievedFromMaster = false;
	m_bNoServersListedOnMaster = false;
	m_bAnyServersRespondedToQuery = false;

	LoadRegionSettings();
	LoadFilterSettings();
}

CInternetGames::~CInternetGames(void)
{
}

void CInternetGames::LoadRegionSettings(void)
{
	if (m_pLocationFilter->GetItemCount() > 0)
		m_pLocationFilter->DeleteAllItems();

	KeyValues *kv = new KeyValues("Regions");

	if (kv->LoadFromFile(g_pFullFileSystem, "servers/Regions.vdf", NULL))
	{
		for (KeyValues *srv = kv->GetFirstSubKey(); srv != NULL; srv = srv->GetNextKey())
		{
			struct regions_s region;
			region.name = srv->GetString("text");
			region.code = srv->GetInt("code");

			KeyValues *regionKV = new KeyValues("region", "code", region.code);
			m_pLocationFilter->AddItem(region.name.String(), regionKV);
			regionKV->deleteThis();
			m_Regions.AddToTail(region);
		}
	}
	else
	{
		Assert(!("Could not load file servers/Regions.vdf; server browser will not function."));
	}

	kv->deleteThis();
}

void CInternetGames::PerformLayout(void)
{
	if (!m_bOfflineMode && m_bRequireUpdate && ServerBrowserDialog().IsVisible())
	{
		PostMessage(this, new KeyValues("GetNewServerList"), 0.1f);
		m_bRequireUpdate = false;
	}

	if (m_bOfflineMode)
	{
		m_pGameList->SetEmptyListText("#ServerBrowser_OfflineMode");
		m_pConnect->SetEnabled(false);
		m_pRefreshAll->SetEnabled(false);
		m_pRefreshQuick->SetEnabled(false);
		m_pAddServer->SetEnabled(false);
		m_pFilter->SetEnabled(false);
	}

	BaseClass::PerformLayout();

	m_pLocationFilter->SetEnabled(true);
}

void CInternetGames::OnPageShow(void)
{
}

void CInternetGames::OnTick(void)
{
	if (m_bOfflineMode)
	{
		BaseClass::OnTick();
		return;
	}

	BaseClass::OnTick();

	CheckRedoSort();
}

void CInternetGames::ServerResponded(HServerListRequest hRequest, int iServer)
{
	m_bDirty = true;

	BaseClass::ServerResponded(hRequest, iServer);

	m_bAnyServersRespondedToQuery = true;
	m_bAnyServersRetrievedFromMaster = true;
}

void CInternetGames::ServerFailedToRespond(HServerListRequest hRequest, int iServer)
{
	m_bDirty = true;

	gameserveritem_t *pServer = SteamMatchmakingServers()->GetServerDetails(hRequest, iServer);
	Assert(pServer);

	if (pServer->m_bHadSuccessfulResponse)
	{
		ServerResponded(hRequest, iServer);
	}
	else
	{
		int iServerMap = m_mapServers.Find(iServer);

		if (iServerMap != m_mapServers.InvalidIndex())
			RemoveServer(m_mapServers[iServerMap]);

		m_iServerRefreshCount++;
	}
}

void CInternetGames::RefreshComplete(HServerListRequest hRequest, EMatchMakingServerResponse response)
{
	SetRefreshing(false);
	UpdateFilterSettings();

	if (response != eServerFailedToRespond)
	{
		if (m_bAnyServersRespondedToQuery)
		{
			m_pGameList->SetEmptyListText(GetStringNoUnfilteredServers());
		}
		else if (response == eNoServersListedOnMasterServer)
		{
			m_pGameList->SetEmptyListText(GetStringNoUnfilteredServersOnMaster());
		}
		else
		{
			m_pGameList->SetEmptyListText(GetStringNoServersResponded());
		}
	}
	else
	{
		m_pGameList->SetEmptyListText("#ServerBrowser_MasterServerNotResponsive");
	}

	m_bDirty = false;
	m_fLastSort = system()->GetCurrentTime();

	if (IsVisible())
	{
		m_pGameList->SortList();
	}

	UpdateStatus();
}

void CInternetGames::GetNewServerList(void)
{
	BaseClass::GetNewServerList();
	UpdateStatus();

	m_bRequireUpdate = false;
	m_bAnyServersRetrievedFromMaster = false;
	m_bAnyServersRespondedToQuery = false;

	m_pGameList->DeleteAllItems();
}

bool CInternetGames::SupportsItem(IGameList::InterfaceItem_e item)
{
	switch (item)
	{
		case FILTERS:
		case GETNEWLIST: return true;
	}

	return false;
}

void CInternetGames::CheckRedoSort(void)
{
	float fCurTime;

	if (!m_bDirty)
		return;

	fCurTime = system()->GetCurrentTime();

	if (fCurTime - m_fLastSort < MINIMUM_SORT_TIME)
		return;

	if (input()->IsMouseDown(MOUSE_LEFT) || input()->IsMouseDown(MOUSE_RIGHT))
	{
		m_fLastSort = fCurTime - MINIMUM_SORT_TIME + 1.0f;
		return;
	}

	m_bDirty = false;
	m_fLastSort = fCurTime;

	m_pGameList->SortList();
}

void CInternetGames::OnOpenContextMenu(int itemID)
{
	if (!m_pGameList->GetSelectedItemsCount())
		return;

	int serverID = m_pGameList->GetItemData(m_pGameList->GetSelectedItem(0))->userData;

	CServerContextMenu *menu = ServerBrowserDialog().GetContextMenu(m_pGameList);
	menu->ShowMenu(this, serverID, true, true, true, true);
}

void CInternetGames::OnRefreshServer(int serverID)
{
	BaseClass::OnRefreshServer(serverID);

	ServerBrowserDialog().UpdateStatusText("#ServerBrowser_GettingNewServerList");
}

int CInternetGames::GetRegionCodeToFilter(void)
{
	KeyValues *kv = m_pLocationFilter->GetActiveItemUserData();

	if (kv)
		return kv->GetInt("code");
	else
		return 255;
}

bool CInternetGames::CheckTagFilter(gameserveritem_t &server)
{
	bool bOfficialServer = !(server.m_szGameTags && server.m_szGameTags[0]);

	if (!bOfficialServer)
		return false;

	return true;
}
