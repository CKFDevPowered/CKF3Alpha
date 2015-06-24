#ifndef BASEGAMESPAGE_H
#define BASEGAMESPAGE_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/ListPanel.h>
#include <vgui_controls/PropertyPage.h>
#include "IGameList.h"
#include <tier1/utlmap.h>
#include "netadr.h"
#include "isteammatchmaking.h"
#include "steam_api.h"

class CBaseGamesPage;

class CGameListPanel : public vgui::ListPanel
{
public:
	DECLARE_CLASS_SIMPLE(CGameListPanel, vgui::ListPanel);

public:
	CGameListPanel(CBaseGamesPage *pOuter, const char *pName);

public:
	virtual void OnKeyCodeTyped(vgui::KeyCode code);

private:
	CBaseGamesPage *m_pOuter;
};

class CBaseGamesPage : public vgui::PropertyPage, public IGameList, public ISteamMatchmakingServerListResponse, public ISteamMatchmakingPingResponse
{
	DECLARE_CLASS_SIMPLE(CBaseGamesPage, vgui::PropertyPage);

public:
	CBaseGamesPage(vgui::Panel *parent, const char *name, EMatchMakingType eType, const char *pCustomResFilename = NULL);
	~CBaseGamesPage(void);

public:
	virtual void PerformLayout(void);
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

public:
	virtual gameserveritem_t *GetServer(unsigned int serverID);
	virtual void SetRefreshing(bool state);
	virtual void LoadFilterSettings(void);
	virtual void StartRefresh(void);
	virtual void UpdateDerivedLayouts(void);

public:
	int GetSelectedItemsCount(void);

public:
	MESSAGE_FUNC(OnAddToFavorites, "AddToFavorites");

public:
	virtual bool OnGameListEnterPressed(void);
	virtual void OnCommand(const char *command);
	virtual void OnKeyCodePressed(vgui::KeyCode code);
	virtual void OnSaveFilter(KeyValues *filter);
	virtual void OnLoadFilter(KeyValues *filter);
	virtual void OnPageShow(void);
	virtual void OnPageHide(void);

protected:
	MESSAGE_FUNC(OnItemSelected, "ItemSelected");
	MESSAGE_FUNC(OnBeginConnect, "ConnectToServer");
	MESSAGE_FUNC(OnViewGameInfo, "ViewGameInfo");
	MESSAGE_FUNC_INT(OnRefreshServer, "RefreshServer", serverID);
	MESSAGE_FUNC_PTR_CHARPTR(OnTextChanged, "TextChanged", panel, text);
	MESSAGE_FUNC_PTR_INT(OnButtonToggled, "ButtonToggled", panel, state);
	STEAM_CALLBACK(CBaseGamesPage, OnFavoritesMsg, FavoritesListChanged_t, m_CallbackFavoritesMsg);

protected:
	virtual int GetRegionCodeToFilter(void) { return -1; }
	virtual void ServerResponded(HServerListRequest hRequest, int iServer);
	virtual void ServerResponded(int iServer, gameserveritem_t *pServerItem);
	virtual void ServerFailedToRespond(HServerListRequest hRequest, int iServer);
	virtual void RefreshComplete(HServerListRequest hRequest, EMatchMakingServerResponse response) = 0;
	virtual void ServerResponded(gameserveritem_t &server);
	virtual void ServerFailedToRespond(void) {}
	virtual bool BShowServer(serverdisplay_t &server) { return server.m_bDoNotRefresh; } 
	virtual bool CheckPrimaryFilters(gameserveritem_t &server);
	virtual bool CheckSecondaryFilters(gameserveritem_t &server);
	virtual bool CheckTagFilter(gameserveritem_t &server) { return true; }
	virtual int GetInvalidServerListID(void);
	virtual void UpdateFilterSettings(void);
	virtual void GetNewServerList(void);
	virtual void StopRefresh(void);
	virtual bool IsRefreshing(void);
	virtual void CreateFilters(void);
	virtual void ApplyFilters(void);
	virtual void UpdateGameFilter(void);
	virtual void LoadFilter(KeyValues *filter);
	virtual void SaveFilter(KeyValues *filter);

protected:
	void ApplyGameFilters(void);
	void UpdateStatus(void);
	void RemoveServer(serverdisplay_t &server);
	void ClearServerList(void);
	uint32 GetFilterAppID(void) { return m_iLimitToAppID; }
	uint32 GetServerFilters(MatchMakingKeyValuePair_t **pFilters);

protected:
	bool m_bAutoSelectFirstItemInGameList;

	CGameListPanel *m_pGameList;
	vgui::ComboBox *m_pLocationFilter;

	vgui::Button *m_pConnect;
	vgui::Button *m_pRefreshAll;
	vgui::Button *m_pRefreshQuick;
	vgui::Button *m_pAddServer;
	vgui::Button *m_pAddCurrentServer;
	vgui::Button *m_pAddToFavoritesButton;
	vgui::ToggleButton *m_pFilter;

	CUtlMap<int, serverdisplay_t> m_mapServers;
	CUtlMap<netadr_t, int> m_mapServerIP;
	CUtlVector<MatchMakingKeyValuePair_t> m_vecServerFilters;
	int m_iServerRefreshCount;
	HServerListRequest m_hServerRequest;
	EMatchMakingType m_eMatchMakingType;

private:
	void RecalculateFilterString(void);

private:
	const char *m_pCustomResFilename;

	vgui::TextEntry *m_pGameFilter;
	vgui::TextEntry *m_pMapFilter;
	vgui::ComboBox *m_pPingFilter;
	vgui::ComboBox *m_pSecureFilter;
	vgui::CheckButton *m_pNoFullServersFilterCheck;
	vgui::CheckButton *m_pNoEmptyServersFilterCheck;
	vgui::CheckButton *m_pNoPasswordFilterCheck;
	vgui::Label *m_pFilterString;
	char m_szComboAllText[64];

	KeyValues *m_pFilters;
	bool m_bFiltersVisible;
	vgui::HFont m_hFont;

	char m_szGameFilter[32];
	char m_szMapFilter[32];
	int m_iPingFilter;
	bool m_bFilterNoFullServers;
	bool m_bFilterNoEmptyServers;
	bool m_bFilterNoPasswordedServers;
	int m_iSecureFilter;

	int m_iLimitToAppID;
};

#endif
