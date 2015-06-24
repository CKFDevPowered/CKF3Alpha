#ifndef ISERVERGETCHALLENGERESPONSE_H
#define ISERVERGETCHALLENGERESPONSE_H

#ifdef _WIN32
#pragma once
#endif

class IServerGetChallengeResponse
{
public:
	virtual void GetChallengeFailedToRespond(void) = 0;
	virtual void GetChallengeRefreshComplete(int challenge) = 0;
};

#endif