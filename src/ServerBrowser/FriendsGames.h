#ifndef FRIENDSGAMES_H
#define FRIENDSGAMES_H
#ifdef _WIN32
#pragma once
#endif

#include "BaseGamesPage.h"

class CFriendsGames : public CBaseGamesPage
{
	DECLARE_CLASS_SIMPLE( CFriendsGames, CBaseGamesPage );

public:
	CFriendsGames(vgui::Panel *parent);
	~CFriendsGames(void);

public:
	virtual bool SupportsItem(InterfaceItem_e item);
	virtual void RefreshComplete(HServerListRequest hRequest, EMatchMakingServerResponse response);

private:
	MESSAGE_FUNC_INT(OnOpenContextMenu, "OpenContextMenu", itemID);

private:
	int m_iServerRefreshCount;
};

#endif