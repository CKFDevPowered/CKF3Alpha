#ifndef ISERVERBROWSER_H
#define ISERVERBROWSER_H

#ifdef _WIN32
#pragma once
#endif

#include "interface.h"

class KeyValues;

namespace vgui
{
	class Panel;
};

class IServerBrowser : public IBaseInterface
{
public:
	virtual bool Initialize(CreateInterfaceFn *factorylist, int numFactories) = 0;
	virtual void Shutdown(void) = 0;
	virtual void SetParent(vgui::VPANEL parent) = 0;
	virtual void Deactivate(void) = 0;
	virtual void Reactivate(void) = 0;
	virtual bool Activate(void) = 0;
	virtual void ActiveGameName(const char *szGameName, const char *szGameDir) = 0;
	virtual void ConnectToGame(int ip, int connectionport) = 0;
	virtual void DisconnectFromGame(void) = 0;
	virtual bool JoinGame(uint32 unGameIP, uint16 usGamePort) = 0;
	virtual bool JoinGame(uint64 ulSteamIDFriend) = 0;
	virtual bool OpenGameInfoDialog(uint64 ulSteamIDFriend) = 0;
	virtual void CloseGameInfoDialog(uint64 ulSteamIDFriend) = 0;
	virtual void CloseAllGameInfoDialogs(void) = 0;
};

#define SERVERBROWSER_INTERFACE_VERSION "ServerBrowser003"

#endif