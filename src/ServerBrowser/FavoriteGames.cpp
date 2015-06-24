#include <time.h>

#include "FavoriteGames.h"
#include "ServerContextMenu.h"
#include "ServerListCompare.h"
#include "ServerBrowserDialog.h"
#include "DialogAddServer.h"
#include "InternetGames.h"
#include "FileSystem.h"
#include "UtlBuffer.h"

#include <vgui/IScheme.h>
#include <vgui/ISystem.h>
#include <vgui/IVGui.h>
#include <KeyValues.h>

#include <vgui_controls/Button.h>
#include <vgui_controls/ListPanel.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/MessageBox.h>

using namespace vgui;

CFavoriteGames::CFavoriteGames(vgui::Panel *parent) : CBaseGamesPage(parent, "FavoriteGames", eFavoritesServer)
{
	m_bRefreshOnListReload = false;
}

CFavoriteGames::~CFavoriteGames(void)
{
}

void CFavoriteGames::LoadFavoritesList(void)
{
	if (SteamMatchmaking() && SteamMatchmaking()->GetFavoriteGameCount() == 0)
	{
		m_pGameList->SetEmptyListText("#ServerBrowser_NoFavoriteServers");
	}
	else
	{
		m_pGameList->SetEmptyListText("#ServerBrowser_NoInternetGamesResponded");
	}

	if (m_bRefreshOnListReload)
	{
		m_bRefreshOnListReload = false;
		StartRefresh();
	}
}

bool CFavoriteGames::SupportsItem(InterfaceItem_e item)
{
	switch (item)
	{
		case FILTERS:
		case ADDSERVER: return true;
	}

	return false;
}


void CFavoriteGames::RefreshComplete(HServerListRequest hRequest, EMatchMakingServerResponse response)
{
	SetRefreshing(false);

	if (SteamMatchmaking() && SteamMatchmaking()->GetFavoriteGameCount() == 0)
	{
		m_pGameList->SetEmptyListText("#ServerBrowser_NoFavoriteServers");
	}
	else
	{
		m_pGameList->SetEmptyListText("#ServerBrowser_NoInternetGamesResponded");

	}

	m_pGameList->SortList();
}

void CFavoriteGames::OnOpenContextMenu(int itemID)
{
	CServerContextMenu *menu = ServerBrowserDialog().GetContextMenu(m_pGameList);

	if (m_pGameList->GetSelectedItemsCount())
	{
		int serverID = m_pGameList->GetItemUserData(m_pGameList->GetSelectedItem(0));

		menu->ShowMenu(this, serverID, true, true, true, false);
		menu->AddMenuItem("RemoveServer", "#ServerBrowser_RemoveServerFromFavorites", new KeyValues("RemoveFromFavorites"), this);
	}
	else
	{
		menu->ShowMenu(this,(uint32)-1, false, false, false, false);
	}

	menu->AddMenuItem("AddServerByName", "#ServerBrowser_AddServerByIP", new KeyValues("AddServerByName"), this);
}

void CFavoriteGames::OnRemoveFromFavorites(void)
{
	if (!SteamMatchmakingServers() || !SteamMatchmaking())
		return;

	for (int iGame = 0; iGame < m_pGameList->GetSelectedItemsCount(); iGame++)
	{
		int itemID = m_pGameList->GetSelectedItem(iGame);
		int serverID = m_pGameList->GetItemData(itemID)->userData;

		gameserveritem_t *pServer = SteamMatchmakingServers()->GetServerDetails(m_hServerRequest, serverID);

		if (pServer)
		{
			SteamMatchmaking()->RemoveFavoriteGame(pServer->m_nAppID, pServer->m_NetAdr.GetIP(), pServer->m_NetAdr.GetConnectionPort(), pServer->m_NetAdr.GetQueryPort(), k_unFavoriteFlagFavorite);
		}
	}

	UpdateStatus();
	InvalidateLayout();
	Repaint();
}

void CFavoriteGames::OnAddServerByName(void)
{
	CDialogAddServer *dlg = new CDialogAddServer(&ServerBrowserDialog(), this);
	dlg->MoveToCenterOfScreen();
	dlg->DoModal();
}

void CFavoriteGames::OnAddCurrentServer(void)
{
	gameserveritem_t *pConnected = ServerBrowserDialog().GetCurrentConnectedServer();

	if (pConnected && SteamMatchmaking())
	{
		SteamMatchmaking()->AddFavoriteGame(pConnected->m_nAppID, pConnected->m_NetAdr.GetIP(), pConnected->m_NetAdr.GetConnectionPort(), pConnected->m_NetAdr.GetQueryPort(), k_unFavoriteFlagFavorite, time(NULL));
		m_bRefreshOnListReload = true;
	}
}

void CFavoriteGames::OnCommand(const char *command)
{
	if (!Q_stricmp(command, "AddServerByName"))
	{
		OnAddServerByName();
	}
	else if (!Q_stricmp(command, "AddCurrentServer" ))
	{
		OnAddCurrentServer();
	}
	else
	{
		BaseClass::OnCommand(command);
	}
}

void CFavoriteGames::OnConnectToGame(void)
{
	m_pAddCurrentServer->SetEnabled(true);
}

void CFavoriteGames::OnDisconnectFromGame(void)
{
	m_pAddCurrentServer->SetEnabled(false);
}