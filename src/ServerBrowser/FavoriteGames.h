#ifndef FAVORITEGAMES_H
#define FAVORITEGAMES_H
#ifdef _WIN32
#pragma once
#endif

#include "BaseGamesPage.h"

class CFavoriteGames : public CBaseGamesPage
{
	DECLARE_CLASS_SIMPLE(CFavoriteGames, CBaseGamesPage);

public:
	CFavoriteGames(vgui::Panel *parent);
	~CFavoriteGames(void);

public:
	virtual bool SupportsItem(InterfaceItem_e item);
	virtual void RefreshComplete(HServerListRequest hRequest, EMatchMakingServerResponse response);

public:
	void OnConnectToGame(void);
	void OnDisconnectFromGame(void);

public:
	void LoadFavoritesList(void);
	void SetRefreshOnReload(void) { m_bRefreshOnListReload = true; }

private:
	MESSAGE_FUNC_INT(OnOpenContextMenu, "OpenContextMenu", itemID);
	MESSAGE_FUNC(OnRemoveFromFavorites, "RemoveFromFavorites");
	MESSAGE_FUNC(OnAddServerByName, "AddServerByName");

private:
	void OnAddCurrentServer(void);
	void OnCommand(const char *command);

private:
	bool m_bRefreshOnListReload;
};

#endif