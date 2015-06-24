#include "HistoryGames.h"

#include "ServerContextMenu.h"
#include "ServerListCompare.h"
#include "ServerBrowserDialog.h"
#include "InternetGames.h"
#include "FileSystem.h"
#include "UtlBuffer.h"
#include "OfflineMode.h"

#include <vgui/IScheme.h>
#include <vgui/ISystem.h>
#include <vgui/IVGui.h>
#include <KeyValues.h>

#include <vgui_controls/Button.h>
#include <vgui_controls/ListPanel.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/MessageBox.h>
#include <vgui_controls/ToggleButton.h>

using namespace vgui;

CHistoryGames::CHistoryGames(vgui::Panel *parent) : CBaseGamesPage(parent, "HistoryGames", eHistoryServer)
{
	m_bRefreshOnListReload = false;
	m_pGameList->AddColumnHeader(7, "LastPlayed", "#ServerBrowser_LastPlayed", 100);
	m_pGameList->SetSortFunc(7, LastPlayedCompare);
	m_pGameList->SetSortColumn(7);

	if (!IsSteamGameServerBrowsingEnabled())
	{
		m_pGameList->SetEmptyListText("#ServerBrowser_OfflineMode");
		m_pConnect->SetEnabled(false);
		m_pRefreshAll->SetEnabled(false);
		m_pRefreshQuick->SetEnabled(false);
		m_pAddServer->SetEnabled(false);
		m_pFilter->SetEnabled(false);
	}
}

CHistoryGames::~CHistoryGames(void)
{
}

void CHistoryGames::LoadHistoryList(void)
{
	if (IsSteamGameServerBrowsingEnabled())
	{
		m_pGameList->SetEmptyListText("#ServerBrowser_NoServersPlayed");
	}

	if (m_bRefreshOnListReload)
	{
		m_bRefreshOnListReload = false;
		StartRefresh();
	}
}

bool CHistoryGames::SupportsItem(InterfaceItem_e item)
{
	switch (item)
	{
		case FILTERS: return true;
	}

	return false;
}

void CHistoryGames::RefreshComplete(HServerListRequest hRequest, EMatchMakingServerResponse response)
{
	SetRefreshing(false);
	m_pGameList->SetEmptyListText("#ServerBrowser_NoServersPlayed");
	m_pGameList->SortList();
}

void CHistoryGames::OnOpenContextMenu(int itemID)
{
	CServerContextMenu *menu = ServerBrowserDialog().GetContextMenu(m_pGameList);

	if (m_pGameList->GetSelectedItemsCount())
	{
		int serverID = m_pGameList->GetItemUserData(m_pGameList->GetSelectedItem(0));
		
		menu->ShowMenu(this, serverID, true, true, true, true);
		menu->AddMenuItem("RemoveServer", "#ServerBrowser_RemoveServerFromHistory", new KeyValues("RemoveFromHistory"), this);
	}
	else
	{
		menu->ShowMenu(this, (uint32)-1, false, false, false, false);
	}
}

void CHistoryGames::OnRemoveFromHistory(void)
{
	if (!SteamMatchmakingServers() || !SteamMatchmaking())
		return;

	for (int i = m_pGameList->GetSelectedItemsCount() - 1; i >= 0; i--)
	{
		int itemID = m_pGameList->GetSelectedItem(i);
		int serverID = m_pGameList->GetItemData(itemID)->userData;
	
		gameserveritem_t *pServer = SteamMatchmakingServers()->GetServerDetails(m_hServerRequest, serverID);

		if (pServer)
			SteamMatchmaking()->RemoveFavoriteGame(pServer->m_nAppID, pServer->m_NetAdr.GetIP(), pServer->m_NetAdr.GetConnectionPort(), pServer->m_NetAdr.GetQueryPort(), k_unFavoriteFlagHistory);
	}

	UpdateStatus();	
	InvalidateLayout();
	Repaint();
}