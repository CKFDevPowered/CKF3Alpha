#if !defined(INETAPI_H)
#define INETAPI_H

#ifdef _WIN32
#pragma once
#endif

#include "netadr.h"

class INetAPI
{
public:
	virtual void NetAdrToSockAddr(netadr_t *a, struct sockaddr *s) = 0;
	virtual void SockAddrToNetAdr(struct sockaddr *s, netadr_t *a) = 0;
	virtual char *AdrToString(netadr_t *a) = 0;
	virtual bool StringToAdr(const char *s, netadr_t *a) = 0;
	virtual void GetSocketAddress(int socket, netadr_t *a) = 0;
	virtual bool CompareAdr(netadr_t *a, netadr_t *b) =0;
};

extern INetAPI *net;

#endif