#include "EngineInterface.h"
#include "ServerBrowser.h"
#include "ServerBrowserDialog.h"
#include "DialogGameInfo.h"
#include "steam_api.h"

#include <vgui/ILocalize.h>
#include <vgui/IPanel.h>
#include <vgui/IVGui.h>
#include <KeyValues.h>

CServerBrowser g_ServerBrowserSingleton;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CServerBrowser, IServerBrowser, SERVERBROWSER_INTERFACE_VERSION, g_ServerBrowserSingleton);

CServerBrowser &ServerBrowser(void)
{
	return g_ServerBrowserSingleton;
}

CServerBrowser::CServerBrowser(void)
{
}

CServerBrowser::~CServerBrowser(void)
{
}

bool CServerBrowser::Initialize(CreateInterfaceFn *factorylist, int factoryCount)
{
	if (!vgui::VGui_InitInterfacesList("GameUI", factorylist, factoryCount))
		return false;

	g_pVGuiLocalize->AddFile(g_pFullFileSystem, "servers/serverbrowser_%language%.txt");

	if (!m_hInternetDlg.Get())
	{
		m_hInternetDlg = new CServerBrowserDialog(NULL);
		m_hInternetDlg->Initialize();
	}

	return true;
}

void CServerBrowser::ActiveGameName(const char *szGameName, const char *szGameDir)
{
	m_hInternetDlg->PostMessage(m_hInternetDlg->GetVPanel(), new KeyValues("ActiveGameName", "game", szGameName, "name", szGameDir), 0.0);
}

void CServerBrowser::ConnectToGame(int ip, int connectionport)
{
	m_hInternetDlg->PostMessage(m_hInternetDlg->GetVPanel(), new KeyValues("ConnectedToGame", "ip", ip, "connectionport", connectionport), 0.0);
}

void CServerBrowser::DisconnectFromGame(void)
{
	m_hInternetDlg->PostMessage(m_hInternetDlg->GetVPanel(), new KeyValues("DisconnectedFromGame"), 0.0);
}

bool CServerBrowser::IsVACBannedFromGame(int nAppID)
{
	return false;
}

bool CServerBrowser::IsValid(void)
{
	return true;
}

bool CServerBrowser::Activate(void)
{
	m_hInternetDlg->Open();
	return true;
}

void CServerBrowser::Deactivate(void)
{
	if (m_hInternetDlg.Get())
		m_hInternetDlg->SaveUserData();
}

void CServerBrowser::Reactivate(void)
{
	if (m_hInternetDlg.Get())
	{
		m_hInternetDlg->LoadUserData();

		if (m_hInternetDlg->IsVisible())
			m_hInternetDlg->RefreshCurrentPage();
	}
}

vgui::VPANEL CServerBrowser::GetPanel(void)
{
	return m_hInternetDlg.Get() ? m_hInternetDlg->GetVPanel() : NULL;
}

void CServerBrowser::SetParent(vgui::VPANEL parent)
{
	if (m_hInternetDlg.Get())
		m_hInternetDlg->SetParent(parent);
}

void CServerBrowser::Shutdown(void)
{
	if (m_hInternetDlg.Get())
	{
		m_hInternetDlg->Close();
		m_hInternetDlg->MarkForDeletion();
	}
}

bool CServerBrowser::OpenGameInfoDialog(uint64 ulSteamIDFriend)
{
	if (m_hInternetDlg.Get())
	{
		CDialogGameInfo *pDialogGameInfo = m_hInternetDlg->GetDialogGameInfoForFriend(ulSteamIDFriend);

		if (pDialogGameInfo)
		{
			pDialogGameInfo->Activate();
			return true;
		}

		FriendGameInfo_t gameinfo;

		if (SteamFriends()->GetFriendGamePlayed(ulSteamIDFriend, &gameinfo))
		{
			uint16 usConnPort = gameinfo.m_usGamePort;

			if (gameinfo.m_usQueryPort < QUERY_PORT_ERROR)
				usConnPort = gameinfo.m_usGamePort;

			CDialogGameInfo *pDialogGameInfo = m_hInternetDlg->OpenGameInfoDialog(gameinfo.m_unGameIP, gameinfo.m_usGamePort, usConnPort);
			pDialogGameInfo->SetFriend(ulSteamIDFriend);
			return true;
		}
	}

	return false;
}

bool CServerBrowser::JoinGame(uint64 ulSteamIDFriend)
{
	if (OpenGameInfoDialog(ulSteamIDFriend))
	{
		CDialogGameInfo *pDialogGameInfo = m_hInternetDlg->GetDialogGameInfoForFriend(ulSteamIDFriend);
		pDialogGameInfo->Connect();
	}

	return false;
}

bool CServerBrowser::JoinGame(uint32 unGameIP, uint16 usGamePort)
{
	m_hInternetDlg->JoinGame(unGameIP, usGamePort);
	return true;
}

void CServerBrowser::CloseGameInfoDialog(uint64 ulSteamIDFriend)
{
	CDialogGameInfo *pDialogGameInfo = m_hInternetDlg->GetDialogGameInfoForFriend(ulSteamIDFriend);

	if (pDialogGameInfo)
	{
		pDialogGameInfo->Close();
	}
}

void CServerBrowser::CloseAllGameInfoDialogs(void)
{
	if (m_hInternetDlg.Get())
	{
		m_hInternetDlg->CloseAllGameInfoDialogs();
	}
}