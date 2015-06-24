#ifndef ISERVERPLAYERSRESPONSE_H
#define ISERVERPLAYERSRESPONSE_H

#ifdef _WIN32
#pragma once
#endif

class IServerPlayersResponse
{
public:
	virtual void AddPlayerToList(const char *pchName, int nScore, float flTimePlayed) = 0;
	virtual void PlayersFailedToRespond(void) = 0;
	virtual void PlayersRefreshComplete(void) = 0;
};

#endif