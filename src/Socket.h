#if !defined (SOCKET_H)
#define SOCKET_H

#ifdef _WIN32
#pragma once
#endif

#include "netadr.h"
#include "MsgBuffer.h"
#include "UtlVector.h"

#include <stdio.h>

class CMsgBuffer;
class CSocket;
class IGameList;

#pragma pack(1)

typedef struct
{
	int netID;
	int sequenceNumber;
	char packetID;
}
SPLITPACKET;

#pragma pack()

#define MAX_PACKETS 16
#define MAX_RETRIES 2

class CMsgHandler
{
public:
	enum
	{
		MAX_HANDLER_STRING = 64
	};

	typedef enum
	{
		MSGHANDLER_ALL = 0,
		MSGHANDLER_BYTECODE,
		MSGHANDLER_STRING
	}
	HANDLERTYPE;

	CMsgHandler(HANDLERTYPE type, int typeinfo = 0);
	CMsgHandler(HANDLERTYPE type, void *typeinfo = NULL);
	virtual ~CMsgHandler(void);

public:
	virtual void Init(HANDLERTYPE type, void *typeinfo);
	virtual bool Process(netadr_t *from, CMsgBuffer *msg) = 0;
	virtual CMsgHandler *GetNext(void) const;
	virtual void SetNext(CMsgHandler *next);
	virtual CSocket *GetSocket(void) const;
	virtual void SetSocket(CSocket *socket);

private:
	bool ProcessMessage(netadr_t *from, CMsgBuffer *msg);

private:
	IGameList *m_pBaseObject;

	HANDLERTYPE m_Type;
	unsigned char m_ByteCode;
	char m_szString[MAX_HANDLER_STRING];
	CMsgHandler *m_pNext;
	CSocket *m_pSocket;

private:
	friend CSocket;
};

class CSocket
{
public:
	CSocket(const char *socketname, int port = -1);
	virtual ~CSocket(void);

public:
	virtual void AddMessageHandler(CMsgHandler *handler);
	virtual void RemoveMessageHandler(CMsgHandler *handler);

	virtual int SendMessage(netadr_t *to, CMsgBuffer *msg = NULL);
	virtual int Broadcast(int port, CMsgBuffer *msg = NULL);
	virtual CMsgBuffer *GetSendBuffer(void);
	virtual void Frame(void);
	virtual bool IsValid(void) const;
	virtual const netadr_t *GetAddress(void);

	virtual void SetUserData(unsigned int userData);
	virtual unsigned int GetUserData(void) const;

	virtual int GetSocketNumber(void) const;
	virtual bool ReceiveData(void);
	virtual void Sleep(unsigned int ms);
	static float GetClock(void);

private:
	const char *m_pSocketName;
	bool m_bValid;
	netadr_t m_Address;
	bool m_bResolved;
	CUtlVector<CMsgBuffer> m_MsgBuffers;
	CMsgBuffer m_SendBuffer;
	void *m_pBufferCS;
	CMsgHandler *m_pMessageHandlers;
	int m_Socket;
	unsigned int m_nUserData;
	netadr_t m_ToAddress;
	bool m_bBroadcastSend;

	int m_iTotalPackets;
	int m_iCurrentPackets;
	int m_iSeqNo;
	int m_iRetries;
	CMsgBuffer m_CurPacket[MAX_PACKETS];
};

#endif