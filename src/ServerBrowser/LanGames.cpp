#include "LanGames.h"

#include "ServerContextMenu.h"
#include "ServerListCompare.h"
#include "ServerBrowserDialog.h"
#include "InternetGames.h"

#include <KeyValues.h>
#include <vgui/IScheme.h>
#include <vgui/IVGui.h>
#include <vgui/ISystem.h>

#include <vgui_controls/Button.h>
#include <vgui_controls/ToggleButton.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/ListPanel.h>

using namespace vgui;

const float BROADCAST_LIST_TIMEOUT = 0.4f;

CLanGames::CLanGames(vgui::Panel *parent, bool bAutoRefresh, const char *pCustomResFilename) : CBaseGamesPage(parent, "LanGames", eLANServer, pCustomResFilename)
{
	m_iServerRefreshCount = 0;
	m_bRequesting = false;
	m_bAutoRefresh = bAutoRefresh;
}

CLanGames::~CLanGames(void)
{
}

void CLanGames::OnPageShow(void)
{
	if (m_bAutoRefresh)
		StartRefresh();
}

void CLanGames::OnTick(void)
{
	BaseClass::OnTick();
	CheckRetryRequest();
}

bool CLanGames::SupportsItem(InterfaceItem_e item)
{
	switch (item)
	{
		case FILTERS: return true;
	}

	return false;
}

void CLanGames::StartRefresh(void)
{
	BaseClass::StartRefresh();
	m_fRequestTime = system()->GetCurrentTime();
}

void CLanGames::ManualShowButtons(bool bShowConnect, bool bShowRefreshAll, bool bShowFilter)
{
	m_pConnect->SetVisible(bShowConnect);
	m_pRefreshAll->SetVisible(bShowRefreshAll);
	m_pFilter->SetVisible(bShowFilter);
}

void CLanGames::StopRefresh(void)
{
	BaseClass::StopRefresh();

	m_bRequesting = false;
}

void CLanGames::CheckRetryRequest(void)
{
	if (!m_bRequesting)
		return;

	double curtime = system()->GetCurrentTime();

	if (curtime - m_fRequestTime <= BROADCAST_LIST_TIMEOUT)
	{
		return;
	}

	m_bRequesting = false;
}

void CLanGames::ServerFailedToRespond(int iServer)
{
	int iServerMap = m_mapServers.Find(iServer);

	if (iServerMap != m_mapServers.InvalidIndex())
		RemoveServer(m_mapServers[iServerMap]);
}

void CLanGames::RefreshComplete(HServerListRequest hRequest, EMatchMakingServerResponse response)
{
	SetRefreshing(false);

	m_pGameList->SortList();
	m_iServerRefreshCount = 0;
	m_pGameList->SetEmptyListText("#ServerBrowser_NoLanServers");

	SetEmptyListText();
}

void CLanGames::SetEmptyListText(void)
{
	m_pGameList->SetEmptyListText("#ServerBrowser_NoLanServers");
}

void CLanGames::OnOpenContextMenu(int row)
{
	if (!m_pGameList->GetSelectedItemsCount())
		return;

	int serverID = m_pGameList->GetItemUserData(m_pGameList->GetSelectedItem(0));

	CServerContextMenu *menu = ServerBrowserDialog().GetContextMenu(m_pGameList);
	menu->ShowMenu(this, serverID, true, true, true, false);
}