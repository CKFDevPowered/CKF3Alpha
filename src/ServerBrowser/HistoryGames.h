#ifndef HISTORYGAMES_H
#define HISTORYGAMES_H
#ifdef _WIN32
#pragma once
#endif

#include "BaseGamesPage.h"

class CHistoryGames : public CBaseGamesPage
{
	DECLARE_CLASS_SIMPLE(CHistoryGames, CBaseGamesPage);

public:
	CHistoryGames(vgui::Panel *parent);
	~CHistoryGames(void);

public:
	virtual bool SupportsItem(InterfaceItem_e item);
	virtual void RefreshComplete(HServerListRequest hRequest, EMatchMakingServerResponse response);

public:
	void LoadHistoryList(void);
	void SetRefreshOnReload(void) { m_bRefreshOnListReload = true; }

private:
	MESSAGE_FUNC_INT(OnOpenContextMenu, "OpenContextMenu", itemID);
	MESSAGE_FUNC(OnRemoveFromHistory, "RemoveFromHistory");

private:
	bool m_bRefreshOnListReload;
};

#endif