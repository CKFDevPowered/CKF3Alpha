#ifndef SERVERBROWSER_H
#define SERVERBROWSER_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include "IServerBrowser.h"
#include <vgui_controls/PHandle.h>

class CServerBrowserDialog;

class CServerBrowser : public IServerBrowser
{
public:
	CServerBrowser(void);
	~CServerBrowser(void);

public:
	virtual bool Initialize(CreateInterfaceFn *factorylist, int numFactories);
	virtual vgui::VPANEL GetPanel(void);
	virtual void ActiveGameName(const char *szGameName, const char *szGameDir);
	virtual bool Activate(void);
	virtual bool IsValid(void);
	virtual void ConnectToGame(int ip, int connectionport);
	virtual void DisconnectFromGame(void);
	virtual void Shutdown(void);
	virtual void Deactivate(void);
	virtual void Reactivate(void);
	virtual void SetParent(vgui::VPANEL parent);
	virtual bool JoinGame(uint32 unGameIP, uint16 usGamePort);
	virtual bool JoinGame(uint64 ulSteamIDFriend);
	virtual bool OpenGameInfoDialog(uint64 ulSteamIDFriend);
	virtual void CloseGameInfoDialog(uint64 ulSteamIDFriend);
	virtual void CloseAllGameInfoDialogs(void);

public:
	bool IsVACBannedFromGame(int nAppID);

private:
	vgui::DHANDLE<CServerBrowserDialog> m_hInternetDlg;
};

CServerBrowser &ServerBrowser(void);

#endif