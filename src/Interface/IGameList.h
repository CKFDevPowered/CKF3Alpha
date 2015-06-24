#ifndef IGAMELIST_H
#define IGAMELIST_H

#ifdef _WIN32
#pragma once
#endif

class gameserveritem_t;

typedef enum
{
	SERVERVERSION_SAME_VERSION = 0,
	SERVERVERSION_SERVER_OLD,
	SERVERVERSION_SERVER_NEWER
}
SERVERVERSION;

struct serverdisplay_t
{
	serverdisplay_t(void)
	{
		m_iListID = -1;
		m_iServerID = -1;
		m_bDoNotRefresh = true;
	}

	int m_iListID;
	int m_iServerID;
	bool m_bDoNotRefresh;
	bool operator == (const serverdisplay_t &rhs) const { return rhs.m_iServerID == m_iServerID; }
};

class IGameList
{
public:
	enum InterfaceItem_e
	{
		FILTERS,
		GETNEWLIST,
		ADDSERVER,
		ADDCURRENTSERVER,
	};

public:
	virtual bool SupportsItem(InterfaceItem_e item) = 0;
	virtual void StartRefresh(void) = 0;
	virtual void GetNewServerList(void) = 0;
	virtual void StopRefresh(void) = 0;
	virtual bool IsRefreshing(void) = 0;
	virtual gameserveritem_t *GetServer(unsigned int serverID) = 0;
	virtual int GetInvalidServerListID(void) = 0;

public:
	virtual void OnBeginConnect(void) = 0;
};

#endif