#ifndef INTERNETGAMES_H
#define INTERNETGAMES_H

#ifdef _WIN32
#pragma once
#endif

#include "BaseGamesPage.h"

class CInternetGames : public CBaseGamesPage
{
	DECLARE_CLASS_SIMPLE(CInternetGames, CBaseGamesPage);

public:
	CInternetGames(vgui::Panel *parent, const char *panelName = "InternetGames", EMatchMakingType eType = eInternetServer);
	~CInternetGames(void);

public:
	virtual void OnPageShow(void);
	virtual void OnTick(void);

public:
	virtual bool SupportsItem(IGameList::InterfaceItem_e item);
	virtual void ServerResponded(HServerListRequest hRequest, int iServer);
	virtual void ServerFailedToRespond(HServerListRequest hRequest, int iServer);
	virtual void RefreshComplete(HServerListRequest hRequest, EMatchMakingServerResponse response);
	virtual int GetRegionCodeToFilter(void);
	virtual bool CheckTagFilter(gameserveritem_t &server);

public:
	MESSAGE_FUNC(GetNewServerList, "GetNewServerList");

public:
	virtual const char *GetStringNoUnfilteredServers(void) { return "#ServerBrowser_NoInternetGames"; }
	virtual const char *GetStringNoUnfilteredServersOnMaster(void) { return "#ServerBrowser_MasterServerHasNoServersListed"; }
	virtual const char *GetStringNoServersResponded(void) { return "#ServerBrowser_NoInternetGamesResponded"; }

protected:
	virtual void PerformLayout(void);

private:
	void LoadRegionSettings(void);
	void CheckRedoSort(void);

private:
	MESSAGE_FUNC_INT(OnRefreshServer, "RefreshServer", serverID);
	MESSAGE_FUNC_INT(OnOpenContextMenu, "OpenContextMenu", itemID);

private:
	struct regions_s
	{
		CUtlSymbol name;
		unsigned char code;
	};

	CUtlVector<struct regions_s> m_Regions;

	float m_fLastSort;
	bool m_bDirty;
	bool m_bRequireUpdate;

	bool m_bAnyServersRetrievedFromMaster;
	bool m_bAnyServersRespondedToQuery;
	bool m_bNoServersListedOnMaster;

	bool m_bOfflineMode;
};

#endif