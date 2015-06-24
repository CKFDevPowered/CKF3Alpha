#ifndef LANGAMES_H
#define LANGAMES_H
#ifdef _WIN32
#pragma once
#endif

#include "BaseGamesPage.h"

class CLanGames : public CBaseGamesPage
{
	DECLARE_CLASS_SIMPLE(CLanGames, CBaseGamesPage);

public:
	CLanGames(vgui::Panel *parent, bool bAutoRefresh = true, const char *pCustomResFilename = NULL);
	~CLanGames(void);

public:
	virtual bool SupportsItem(InterfaceItem_e item);
	virtual void StartRefresh(void);
	virtual void StopRefresh(void);
	virtual void ServerFailedToRespond(int iServer);
	virtual void RefreshComplete(HServerListRequest hRequest, EMatchMakingServerResponse response);
	virtual void SetEmptyListText(void);
	virtual void CheckRetryRequest();

public:
	void ManualShowButtons(bool bShowConnect, bool bShowRefreshAll, bool bShowFilter);
	void InternalGetNewServerList(CUtlVector<netadr_t> *pSpecificAddresses);

private:
	virtual void OnPageShow(void);
	virtual void OnTick(void);

private:
	MESSAGE_FUNC_INT(OnOpenContextMenu, "OpenContextMenu", itemID);

private:
	int m_iServerRefreshCount;	
	bool m_bRequesting;
	double m_fRequestTime;
	bool m_bAutoRefresh;
};

#endif