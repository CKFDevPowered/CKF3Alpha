#include "EngineInterface.h"
#include "DialogAddServer.h"
#include "ServerBrowserDialog.h"
#include "ServerBrowser.h"
#include "ServerListCompare.h"
#include "INetAPI.h"
#include "IGameList.h"
#include "steam_api.h"

#include <vgui/IVGui.h>
#include <vgui/ILocalize.h>
#include <KeyValues.h>

#include <vgui_controls/ImageList.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/MessageBox.h>
#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/ListPanel.h>
#include <vgui_controls/TextEntry.h>

using namespace vgui;

CAddServerGameList::CAddServerGameList(Panel *parent, const char *panelName) : BaseClass(parent, panelName)
{
}

void CAddServerGameList::OnKeyCodeTyped(vgui::KeyCode code)
{
	if (!IsInEditMode())
	{
		if (code == KEY_ESCAPE)
		{
			PostMessage(GetParent(), new KeyValues("KeyCodeTyped", "code", code));
			return;
		}

		if (code == KEY_ENTER)
		{
			PostActionSignal(new KeyValues("Command", "command", "addselected"));
			return;
		}
	}

	BaseClass::OnKeyCodeTyped(code);
}

CDialogAddServer::CDialogAddServer(vgui::Panel *parent, IGameList *gameList) : Frame(parent, "DialogAddServer")
{
	SetDeleteSelfOnClose(true);

	m_pGameList = gameList;

	SetTitle("#ServerBrowser_AddServersTitle", true);
	SetSizeable(false);

	m_pTabPanel = new PropertySheet(this, "GameTabs");
	m_pTabPanel->SetTabWidth(72);

	m_pDiscoveredGames = new CAddServerGameList(this, "Servers");

	m_pDiscoveredGames->AddColumnHeader(0, "Password", "#ServerBrowser_Password", 16, ListPanel::COLUMN_FIXEDSIZE | ListPanel::COLUMN_IMAGE);
	m_pDiscoveredGames->AddColumnHeader(1, "Bots", "#ServerBrowser_Bots", 16, ListPanel::COLUMN_FIXEDSIZE | ListPanel::COLUMN_HIDDEN);
	m_pDiscoveredGames->AddColumnHeader(2, "Secure", "#ServerBrowser_Secure", 16, ListPanel::COLUMN_FIXEDSIZE | ListPanel::COLUMN_IMAGE);
	m_pDiscoveredGames->AddColumnHeader(3, "Name", "#ServerBrowser_Servers", 30, ListPanel::COLUMN_RESIZEWITHWINDOW | ListPanel::COLUMN_UNHIDABLE);
	m_pDiscoveredGames->AddColumnHeader(4, "IPAddr", "#ServerBrowser_IPAddress", 60, ListPanel::COLUMN_HIDDEN);
	m_pDiscoveredGames->AddColumnHeader(5, "GameDesc", "#ServerBrowser_Game", 112, 112, 300, 0);
	m_pDiscoveredGames->AddColumnHeader(6, "Players", "#ServerBrowser_Players", 55, ListPanel::COLUMN_FIXEDSIZE);
	m_pDiscoveredGames->AddColumnHeader(7, "Map", "#ServerBrowser_Map", 75, 75, 300, 0);
	m_pDiscoveredGames->AddColumnHeader(8, "Ping", "#ServerBrowser_Latency", 55, ListPanel::COLUMN_FIXEDSIZE);

	m_pDiscoveredGames->SetColumnHeaderTooltip(0, "#ServerBrowser_PasswordColumn_Tooltip");
	m_pDiscoveredGames->SetColumnHeaderTooltip(1, "#ServerBrowser_BotColumn_Tooltip");
	m_pDiscoveredGames->SetColumnHeaderTooltip(2, "#ServerBrowser_SecureColumn_Tooltip");

	m_pDiscoveredGames->SetSortFunc(0, PasswordCompare);
	m_pDiscoveredGames->SetSortFunc(1, BotsCompare);
	m_pDiscoveredGames->SetSortFunc(2, SecureCompare);
	m_pDiscoveredGames->SetSortFunc(3, ServerNameCompare);
	m_pDiscoveredGames->SetSortFunc(4, IPAddressCompare);
	m_pDiscoveredGames->SetSortFunc(5, GameCompare);
	m_pDiscoveredGames->SetSortFunc(6, PlayersCompare);
	m_pDiscoveredGames->SetSortFunc(7, MapCompare);
	m_pDiscoveredGames->SetSortFunc(8, PingCompare);

	m_pDiscoveredGames->SetSortColumn(8);

	m_pTextEntry = new vgui::TextEntry(this, "ServerNameText");
	m_pTextEntry->AddActionSignalTarget(this);

	m_pTestServersButton = new vgui::Button(this, "TestServersButton", "");
	m_pAddServerButton = new vgui::Button(this, "OKButton", "");
	m_pAddSelectedServerButton = new vgui::Button(this, "SelectedOKButton", "", this, "addselected");
	m_pTabPanel->AddPage(m_pDiscoveredGames, "#ServerBrowser_Servers");

	LoadControlSettings("Servers/DialogAddServer.res");

	m_pAddServerButton->SetEnabled(false);
	m_pTestServersButton->SetEnabled(false);
	m_pAddSelectedServerButton->SetEnabled(false);
	m_pAddSelectedServerButton->SetVisible(false);
	m_pTabPanel->SetVisible(false);

	m_pTextEntry->RequestFocus();

	int x, y;
	m_pTabPanel->GetPos(x, y);
	m_OriginalHeight = m_pTabPanel->GetTall() + y + 50;
	SetTall(y);
}

CDialogAddServer::~CDialogAddServer(void)
{
	FOR_EACH_VEC(m_Queries, i)
	{
		if (SteamMatchmakingServers())
			SteamMatchmakingServers()->CancelServerQuery(m_Queries[i]);
	}
}

void CDialogAddServer::OnTextChanged(void)
{
	bool bAnyText = (m_pTextEntry->GetTextLength() > 0);

	m_pAddServerButton->SetEnabled(bAnyText);
	m_pTestServersButton->SetEnabled(bAnyText);
}

void CDialogAddServer::OnCommand(const char *command)
{
	if (!Q_stricmp(command, "OK"))
	{
		OnOK();
	}
	else if (!Q_stricmp(command, "TestServers"))
	{
		SetTall(m_OriginalHeight);

		m_pTabPanel->SetVisible(true);
		m_pAddSelectedServerButton->SetVisible(true);

		TestServers();
	}
	else if (!Q_stricmp(command, "addselected"))
	{
		for (int i = 0; i < m_pDiscoveredGames->GetSelectedItemsCount(); i++)
		{
			int itemID = m_pDiscoveredGames->GetSelectedItem(i);
			int serverID = m_pDiscoveredGames->GetItemUserData(itemID);

			m_pDiscoveredGames->RemoveItem(itemID);
			ServerBrowserDialog().AddServerToFavorites(m_Servers[serverID]);
		}

		m_pDiscoveredGames->SetEmptyListText("");
	}
	else
	{
		BaseClass::OnCommand(command);
	}
}

void CDialogAddServer::OnOK(void)
{
	const char *address = GetControlString("ServerNameText", "");

	netadr_t netaddr;
	netaddr.SetFromString(address, true);

	if (!netaddr.GetPort())
	{
		netaddr.SetPort(27015);
	}

	if (netaddr.IsValid())
	{
		gameserveritem_t server;
		memset(&server, 0, sizeof(server));
		server.SetName(address);

		server.m_NetAdr.Init(netaddr.addr_ntohl(), netaddr.GetPort(), netaddr.GetPort());
		server.m_nAppID = 0;

		ServerBrowserDialog().AddServerToFavorites(server);
	}
	else
	{
		MessageBox *dlg = new MessageBox("#ServerBrowser_AddServerErrorTitle", "#ServerBrowser_AddServerError");
		dlg->DoModal();
	}

	PostMessage(this, new KeyValues("Close"));
}

void CDialogAddServer::OnKeyCodeTyped(KeyCode code)
{
	if (code == KEY_ESCAPE)
	{
		Close();
	}
	else
	{
		BaseClass::OnKeyCodeTyped(code);
	}
}

void CDialogAddServer::TestServers(void)
{
	if (!SteamMatchmakingServers())
		return;

	m_Servers.RemoveAll();
	m_pDiscoveredGames->SetEmptyListText("");

	const char *address = GetControlString("ServerNameText", "");

	netadr_t netaddr;
	net->StringToAdr(address, &netaddr);

	CUtlVector<netadr_t> vecAdress;

	if (netaddr.port == 0)
	{
		CUtlVector<uint16> portsToTry;
		GetMostCommonQueryPorts(portsToTry);

		for (int i = 0; i < portsToTry.Count(); i++)
		{
			netadr_t newAddr = netaddr;
			newAddr.port = portsToTry[i];
			vecAdress.AddToTail(newAddr);
		}
	}
	else
	{
		vecAdress.AddToTail(netaddr);
	}

	m_pTabPanel->RemoveAllPages();

	wchar_t wstr[512];

	if (address[0] == 0)
	{
		Q_wcsncpy(wstr, g_pVGuiLocalize->Find("#ServerBrowser_ServersRespondingLocal"), sizeof(wstr));
	}
	else
	{
		wchar_t waddress[512];
		Q_UTF8ToUnicode(address, waddress, sizeof(waddress));
		g_pVGuiLocalize->ConstructString(wstr, sizeof(wstr), g_pVGuiLocalize->Find( "#ServerBrowser_ServersResponding"), 1, waddress);
	}

	char str[512];
	Q_UnicodeToUTF8(wstr, str, sizeof(str));
	m_pTabPanel->AddPage(m_pDiscoveredGames, str);
	m_pTabPanel->InvalidateLayout();

	FOR_EACH_VEC(vecAdress, iAddress)
	{
		m_Queries.AddToTail(SteamMatchmakingServers()->PingServer(vecAdress[iAddress].addr_htonl(), vecAdress[iAddress].port, this));
	}
}

void CDialogAddServer::ServerResponded(gameserveritem_t &server)
{
	if (server.m_bSecure)
		return;

	KeyValues *kv = new KeyValues("Server");

	kv->SetString("name", server.GetName());
	kv->SetString("map", server.m_szMap);
	kv->SetString("GameDir", server.m_szGameDir);
	kv->SetString("GameDesc", server.m_szGameDescription);
	kv->SetInt("password", server.m_bPassword ? 1 : 0);
	kv->SetInt("bots", server.m_nBotPlayers ? 2 : 0);

	if (server.m_bSecure)
	{
		kv->SetInt("secure", ServerBrowser().IsVACBannedFromGame(server.m_nAppID) ? 4 : 3);
	}
	else
	{
		kv->SetInt("secure", 0);
	}

	netadr_t reportedIPAddr;
	reportedIPAddr.SetIP(server.m_NetAdr.GetIP());
	reportedIPAddr.SetPort(server.m_NetAdr.GetConnectionPort());
	kv->SetString("IPAddr", reportedIPAddr.ToString());

	char buf[32];
	Q_snprintf(buf, sizeof(buf), "%d / %d", server.m_nPlayers, server.m_nMaxPlayers);
	kv->SetString("Players", buf);

	kv->SetInt("Ping", server.m_nPing);

	int iServer = m_Servers.AddToTail(server);
	int iListID = m_pDiscoveredGames->AddItem(kv, iServer, false, false);

	if (m_pDiscoveredGames->GetItemCount() == 1)
	{
		m_pDiscoveredGames->AddSelectedItem(iListID);
	}

	kv->deleteThis();
}

void CDialogAddServer::ServerFailedToRespond(void)
{
	m_pDiscoveredGames->SetEmptyListText("#ServerBrowser_ServerNotResponding");
}

void CDialogAddServer::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	ImageList *imageList = new ImageList(false);
	imageList->AddImage(scheme()->GetImage("resource/servers/icon_password", false));
	imageList->AddImage(scheme()->GetImage("resource/servers/icon_bots", false));
	imageList->AddImage(scheme()->GetImage("resource/servers/icon_robotron", false));
	imageList->AddImage(scheme()->GetImage("resource/servers/icon_secure_deny", false));

	int passwordColumnImage = imageList->AddImage(scheme()->GetImage("resource/servers/icon_password_column", false));
	int botColumnImage = imageList->AddImage(scheme()->GetImage("resource/servers/icon_bots_column", false));
	int secureColumnImage = imageList->AddImage(scheme()->GetImage("resource/servers/icon_robotron_column", false));

	vgui::HFont hFont = pScheme->GetFont("ListSmall", IsProportional());

	if (!hFont)
		hFont = pScheme->GetFont("DefaultSmall", IsProportional());

	m_pDiscoveredGames->SetFont(hFont);
	m_pDiscoveredGames->SetImageList(imageList, true);
	m_pDiscoveredGames->SetColumnHeaderImage(0, passwordColumnImage);
	m_pDiscoveredGames->SetColumnHeaderImage(1, botColumnImage);
	m_pDiscoveredGames->SetColumnHeaderImage(2, secureColumnImage);
}

void CDialogAddServer::OnItemSelected(void)
{
	int nSelectedItem = m_pDiscoveredGames->GetSelectedItem(0);

	if (nSelectedItem != -1)
	{
		m_pAddSelectedServerButton->SetEnabled(true);
	}
	else
	{
		m_pAddSelectedServerButton->SetEnabled(false);
	}
}