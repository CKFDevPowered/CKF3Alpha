#include "FriendsGames.h"

#include "ServerContextMenu.h"
#include "ServerListCompare.h"
#include "ServerBrowserDialog.h"
#include "InternetGames.h"
#include "OfflineMode.h"

#include <KeyValues.h>
#include <vgui/IScheme.h>
#include <vgui/IVGui.h>

#include <vgui_controls/ListPanel.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/ToggleButton.h>

using namespace vgui;

CFriendsGames::CFriendsGames(vgui::Panel *parent) : CBaseGamesPage(parent, "FriendsGames", eFriendsServer)
{
	m_iServerRefreshCount = 0;

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

CFriendsGames::~CFriendsGames(void)
{
}

bool CFriendsGames::SupportsItem(InterfaceItem_e item)
{
	switch (item)
	{
		case FILTERS: return true;
	}

	return false;
}

void CFriendsGames::RefreshComplete(HServerListRequest hRequest, EMatchMakingServerResponse response)
{
	SetRefreshing(false);

	m_pGameList->SortList();
	m_iServerRefreshCount = 0;

	if (IsSteamGameServerBrowsingEnabled())
	{
		m_pGameList->SetEmptyListText("#ServerBrowser_NoFriendsServers");
	}
}

void CFriendsGames::OnOpenContextMenu(int itemID)
{
	if (!m_pGameList->GetSelectedItemsCount())
		return;

	int serverID = m_pGameList->GetItemData(m_pGameList->GetSelectedItem(0))->userData;

	CServerContextMenu *menu = ServerBrowserDialog().GetContextMenu(m_pGameList);
	menu->ShowMenu(this, serverID, true, true, true, true);
}