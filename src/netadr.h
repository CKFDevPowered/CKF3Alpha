#ifndef NETADR_H
#define NETADR_H

#ifdef _WIN32
#pragma once
#endif

#include "steam/steamtypes.h"

#ifdef SetPort
#undef SetPort
#endif

typedef enum
{
	NA_UNUSED,
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP,
	NA_IPX,
	NA_BROADCAST_IPX,
}
netadrtype_t;

typedef struct netadr_s
{
public:
	netadr_s(void) { SetIP(0); SetPort(0); SetType(NA_IP); }
	netadr_s(uint32 unIP, uint16 usPort) { SetIP(unIP); SetPort(usPort); SetType(NA_IP); }
	netadr_s(const char *pch) { SetFromString(pch); }

public:
	void Clear(void);

	void SetType(netadrtype_t type);
	void SetPort(unsigned short port);
	bool SetFromSockadr(const struct sockaddr *s);
	void SetIP(uint8 b1, uint8 b2, uint8 b3, uint8 b4);
	void SetIP(uint32 unIP);
	void SetIPAndPort(uint32 unIP, unsigned short usPort) { SetIP(unIP); SetPort(usPort); }
	void SetFromString(const char *pch, bool bUseDNS = false );

	bool CompareAdr(const netadr_s &a, bool onlyBase = false) const;
	bool CompareClassBAdr(const netadr_s &a) const;
	bool CompareClassCAdr(const netadr_s &a) const;

	netadrtype_t GetType(void) const;
	unsigned short GetPort(void) const;
	const char *ToString(bool onlyBase = false) const;
	void ToSockadr(struct sockaddr *s) const;
	unsigned int GetIP(void) const;

	bool IsLocalhost(void) const;
	bool IsLoopback(void) const;
	bool IsReservedAdr(void) const;
	bool IsValid(void) const;
	void SetFromSocket(int hSocket);

 	unsigned long addr_ntohl(void) const;
 	unsigned long addr_htonl(void) const;
	bool operator == (const netadr_s &netadr) const { return (CompareAdr(netadr)); }
	bool operator < (const netadr_s &netadr) const;

public:
	netadrtype_t type;
	unsigned char ip[4];
	unsigned char ipx[10];
	unsigned short port;
}
netadr_t;

#endif