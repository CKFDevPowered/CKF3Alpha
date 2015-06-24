#define FD_SETSIZE 1024

#include <assert.h>
#include "winlite.h"
#include "winsock.h"
#include "msgbuffer.h"
#include "socket.h"
#include "inetapi.h"

#pragma comment(lib, "ws2_32.lib")

class CSocketThread
{
public:
	typedef struct threadsocket_s
	{
		struct threadsocket_s *next;
		CSocket *socket;
	}
	threadsocket_t;

public:
	CSocketThread(void);
	virtual ~CSocketThread(void);

public:
	virtual void AddSocketToThread(CSocket *socket);
	virtual void RemoveSocketFromThread(CSocket *socket);

	virtual void Lock(void);
	virtual void Unlock(void);

	virtual HANDLE GetShutdownHandle(void);
	virtual threadsocket_t *GetSocketList(void);

	virtual double GetClock(void);

private:
	void InitTimer(void);

private:
	CRITICAL_SECTION cs;
	threadsocket_t *m_pSocketList;
	HANDLE m_hThread;
	DWORD m_nThreadId;
	HANDLE m_hShutdown;

	double m_dClockFrequency;
	double m_dCurrentTime;
	int m_nTimeSampleShift;
	unsigned int m_uiPreviousTime;
};

static CSocketThread *GetSocketThread(void)
{
	static CSocketThread g_SocketThread;
	return &g_SocketThread;
}

static DWORD WINAPI SocketThreadFunc(LPVOID threadobject)
{
	CSocketThread *socketthread = (CSocketThread *)threadobject;

	if (!socketthread)
		return 0;

	while (1)
	{
		CSocketThread::threadsocket_t *sockets;
		fd_set fdset;
		int number;
		int count;

		if (WAIT_OBJECT_0 == WaitForSingleObject(socketthread->GetShutdownHandle(), 0))
			break;

		socketthread->Lock();

		FD_ZERO(&fdset);
		count = 0;

		for (sockets = socketthread->GetSocketList(); sockets; sockets = sockets->next)
		{
			FD_SET(static_cast<u_int>(sockets->socket->GetSocketNumber()), &fdset);
			count = max(count, sockets->socket->GetSocketNumber());
		}

		socketthread->Unlock();

		if (count)
		{
			struct timeval tv;
			tv.tv_sec = 0;
			tv.tv_usec = 2000;

			number = select(count + 1, &fdset, NULL, NULL, &tv);

			if (number > 0)
			{
				socketthread->Lock();

				for (sockets = socketthread->GetSocketList(); sockets; sockets = sockets->next)
				{
					bool bSet = FD_ISSET(sockets->socket->GetSocketNumber(), &fdset);

					if (bSet)
					{
						while (sockets->socket->ReceiveData())
						{
						}
					}
				}

				socketthread->Unlock();
			}
		}
	}

	ExitThread(0);
	return 0;
}

CSocketThread::CSocketThread(void)
{
	InitTimer();
	InitializeCriticalSection(&cs);

	m_pSocketList = NULL;
	m_hShutdown = CreateEvent(NULL, TRUE, FALSE, NULL);

	assert(m_hShutdown);

	m_hThread = 0;
	m_nThreadId = 0;
}

CSocketThread::~CSocketThread(void)
{
	if (m_hThread)
	{
		SetEvent(m_hShutdown);
		TerminateThread(m_hThread, 0);
		CloseHandle(m_hThread);
	}

	CloseHandle(m_hShutdown);
	DeleteCriticalSection(&cs);
}

void CSocketThread::InitTimer(void)
{
}

double CSocketThread::GetClock(void)
{
	CCycleCount cnt;
	cnt.Sample();
	return (double)cnt.GetSeconds();
}

HANDLE CSocketThread::GetShutdownHandle(void)
{
	return m_hShutdown;
}

CSocketThread::threadsocket_t *CSocketThread::GetSocketList(void)
{
	return m_pSocketList;
}

int socketCount = 0;

void CSocketThread::AddSocketToThread(CSocket *socket)
{
	if (!m_hThread)
	{
		m_hThread = CreateThread(NULL, 0, SocketThreadFunc, (void *)this, 0, &m_nThreadId);

		if (m_hThread)
		{
			SetThreadPriority(m_hThread, THREAD_PRIORITY_LOWEST);
		}
	}

	socketCount++;

	threadsocket_t *p = new threadsocket_t;
	p->socket = socket;

	p->next = m_pSocketList;
	m_pSocketList = p;
}

void CSocketThread::RemoveSocketFromThread(CSocket *socket)
{
	if (!m_hThread)
		return;

	socketCount--;

	if (m_pSocketList)
	{
		threadsocket_t *p, *n;
		p = m_pSocketList;

		if (p->socket == socket)
		{
			m_pSocketList = m_pSocketList->next;
			delete p;
		}
		else
		{
			while (p->next)
			{
				n = p->next;

				if (n->socket == socket)
				{
					p->next = n->next;
					delete n;
					break;
				}

				p = n;
			}
		}
	}

	if (socketCount == 0)
	{
		if (m_hThread)
		{
			SetEvent(m_hShutdown);
			TerminateThread(m_hThread, 0);
			CloseHandle(m_hThread);

			m_hThread = NULL;
		}
	}
}

void CSocketThread::Lock(void)
{
	EnterCriticalSection(&cs);
}

void CSocketThread::Unlock(void)
{
	LeaveCriticalSection(&cs);
}

CMsgHandler::CMsgHandler(HANDLERTYPE type, int typeinfo)
{
	Init(type, &typeinfo);
}

CMsgHandler::CMsgHandler(HANDLERTYPE type, void *typeinfo)
{
	Init(type, typeinfo);
}

CMsgHandler::~CMsgHandler(void)
{
}

void CMsgHandler::Init(HANDLERTYPE type, void *typeinfo)
{
	m_Type = type;
	m_pNext = NULL;

	SetSocket(NULL);

	m_ByteCode = 0;
	m_szString[0] = 0;

	switch (m_Type)
	{
		default:
		case MSGHANDLER_ALL:
		{
			break;
		}

		case MSGHANDLER_BYTECODE:
		{
			m_ByteCode = *(unsigned char *)typeinfo;
			break;
		}

		case MSGHANDLER_STRING:
		{
			strcpy(m_szString, (char *)typeinfo);
			break;
		}
	}
}

bool CMsgHandler::Process(netadr_t *from, CMsgBuffer *msg)
{
	return true;
}

bool CMsgHandler::ProcessMessage(netadr_t *from, CMsgBuffer *msg)
{
	bool bret = false;
	unsigned char ch;
	const char *str;

	switch (m_Type)
	{
		case MSGHANDLER_BYTECODE:
		{
			msg->Push();
			ch = (unsigned char)msg->ReadByte();
			msg->Pop();

			if (ch == m_ByteCode)
				bret = Process(from, msg);

			break;
		}

		case MSGHANDLER_STRING:
		{
			msg->Push();
			str = msg->ReadString();
			msg->Pop();

			if (str && str[0] && !stricmp(m_szString, str))
				bret = Process(from, msg);

			break;
		}

		default:
		case MSGHANDLER_ALL:
		{
			bret = Process(from, msg);
			break;
		}
	}

	return bret;
}

CMsgHandler *CMsgHandler::GetNext(void) const
{
	return m_pNext;
}

void CMsgHandler::SetNext(CMsgHandler *next)
{
	m_pNext = next;
}

CSocket *CMsgHandler::GetSocket(void) const
{
	return m_pSocket;
}

void CMsgHandler::SetSocket(CSocket *socket)
{
	m_pSocket = socket;
}

CSocket::CSocket(const char *socketname, int port) : m_SendBuffer(socketname)
{
	struct sockaddr_in address;
	unsigned long _true = 1;
	int i = 1;

	m_pSocketName = socketname;

	m_bValid = false;
	m_bResolved = false;
	m_pMessageHandlers = NULL;
	m_nUserData = 0;
	m_bBroadcastSend = false;
	m_iTotalPackets = 0;
	m_iCurrentPackets = 0;
	m_iRetries = 0;

	m_pBufferCS = new CRITICAL_SECTION;
	InitializeCriticalSection((CRITICAL_SECTION *)m_pBufferCS);

	m_Socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (m_Socket == -1)
		return;

	if (ioctlsocket(m_Socket, FIONBIO, &_true) == -1)
	{
		closesocket(m_Socket);
		m_Socket = 0;
		return;
	}

	Assert(WSAGetLastError() == 0);

	if (setsockopt(m_Socket, SOL_SOCKET, SO_BROADCAST, (char *)&i, sizeof(i)) == -1)
	{
		closesocket(m_Socket);
		m_Socket = 0;
		return;
	}

	if (port != -1)
	{
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons((short)port);
		address.sin_family = AF_INET;

		if (bind(m_Socket, (struct sockaddr *)&address, sizeof(address)) == -1)
		{
			closesocket(m_Socket);
			m_Socket = 0;
			return;
		}
	}

	m_bValid = true;

	Assert(WSAGetLastError() == 0);
	GetSocketThread()->AddSocketToThread(this);
}

CSocket::~CSocket(void)
{
	DeleteCriticalSection((CRITICAL_SECTION *)m_pBufferCS);
	delete (CRITICAL_SECTION *)m_pBufferCS;

	GetSocketThread()->RemoveSocketFromThread(this);

	if (m_bValid)
	{
		shutdown(m_Socket, 0x01);
		shutdown(m_Socket, 0x02);
		closesocket(m_Socket);
		m_Socket = 0;
	}

	CMsgHandler *handler = m_pMessageHandlers;

	while (handler)
	{
		RemoveMessageHandler(handler);
		delete handler;
		handler = m_pMessageHandlers;
	}

	m_pMessageHandlers = NULL;
}

void CSocket::AddMessageHandler(CMsgHandler *handler)
{
	handler->SetSocket(this);
	handler->SetNext(m_pMessageHandlers);

	m_pMessageHandlers = handler;
}

void CSocket::RemoveMessageHandler(CMsgHandler *handler)
{
	if (!handler)
		return;

	CMsgHandler *list = m_pMessageHandlers;

	if (list == handler)
	{
		m_pMessageHandlers = m_pMessageHandlers->GetNext();
		return;
	}

	while (list)
	{
		if (list->GetNext() == handler)
		{
			list->SetNext(handler->GetNext());
			handler->SetNext(NULL);
			return;
		}

		list = list->GetNext();
	}
}

int CSocket::SendMessage(netadr_t *to, CMsgBuffer *msg)
{
	m_bBroadcastSend = false;
	m_ToAddress = *to;

	if (!m_bValid)
		return 0;

	if (!msg)
		msg = GetSendBuffer();

	struct sockaddr addr;
	net->NetAdrToSockAddr(to, &addr);

	int bytessent = sendto(m_Socket, (const char *)msg->GetData(), msg->GetCurSize(), 0, &addr, sizeof(addr));

	if (bytessent == msg->GetCurSize())
		return bytessent;

	return 0;
}

int CSocket::Broadcast(int port, CMsgBuffer *msg)
{
	m_bBroadcastSend = true;
	memset(&m_ToAddress, 0, sizeof(m_ToAddress));

	if (!m_bValid)
		return 0;

	if (!msg)
		msg = GetSendBuffer();

	struct sockaddr addr;
	netadr_t to;

	to.port = (unsigned short)htons((unsigned short)port);
	to.type = NA_BROADCAST;

	net->NetAdrToSockAddr(&to, &addr);

	int bytessent = sendto(m_Socket, (const char *)msg->GetData(), msg->GetCurSize(), 0, &addr, sizeof(addr));

	if (bytessent == msg->GetCurSize())
		return bytessent;

	return 0;
}

void CSocket::Sleep(unsigned int ms)
{
	::Sleep(ms);
}

CMsgBuffer *CSocket::GetSendBuffer(void)
{
	return &m_SendBuffer;
}

void CSocket::Frame(void)
{
	if (!m_MsgBuffers.Size())
		return;

	EnterCriticalSection((CRITICAL_SECTION *)m_pBufferCS);

	for (int i = 0; i < m_MsgBuffers.Size(); i++)
	{
		CMsgHandler *handler = m_pMessageHandlers;
		netadr_t addr = m_MsgBuffers[i].GetNetAddress();

		while (handler)
		{
			if (handler->ProcessMessage(&addr, &m_MsgBuffers[i]))
				break;

			handler = handler->GetNext();
		}
	}

	m_MsgBuffers.RemoveAll();

	LeaveCriticalSection((CRITICAL_SECTION *)m_pBufferCS);
}

bool CSocket::IsValid(void) const
{
	return m_bValid;
}

float CSocket::GetClock(void)
{
	return GetSocketThread()->GetClock();
}

const netadr_t *CSocket::GetAddress(void)
{
	assert(m_bValid);

	if (!m_bResolved)
	{
		m_bResolved = true;
		net->GetSocketAddress(m_Socket, &m_Address);
	}

	return &m_Address;
}

void CSocket::SetUserData(unsigned int userData)
{
	m_nUserData = userData;
}

unsigned int CSocket::GetUserData(void) const
{
	return m_nUserData;
}

int CSocket::GetSocketNumber(void) const
{
	return m_Socket;
}

bool CSocket::ReceiveData(void)
{
	struct sockaddr from;
	int fromlen;
	int bytes;
	unsigned char buffer[CMsgBuffer::NET_MAXMESSAGE];

	fromlen = sizeof(from);
	bytes = recvfrom(m_Socket, (char *)buffer, CMsgBuffer::NET_MAXMESSAGE, 0, (struct sockaddr *)&from, &fromlen);

	if (bytes == -1)
		return false;

	if (bytes >= CMsgBuffer::NET_MAXMESSAGE)
		return false;

	if (bytes < 4)
		return false;

	float recvTime = GetClock();

	netadr_t addr;
	net->SockAddrToNetAdr(&from, &addr);

	if (*(int *)&buffer[0] == -2)
	{
		int curPacket = 0, offset = 0;
		SPLITPACKET *pak = reinterpret_cast<SPLITPACKET *>(&buffer[0]);

		if (m_iTotalPackets == 0)
		{
			m_iTotalPackets = (pak->packetID & 0x0f);
			m_iSeqNo = pak->sequenceNumber;
			m_iRetries = 0;

			m_iCurrentPackets = 1;
			curPacket = (pak->packetID & 0xf0) >> 4;
		}
		else if (m_iSeqNo == pak->sequenceNumber)
		{
			m_iCurrentPackets++;
			curPacket = (pak->packetID & 0xf0) >> 4;
		}
		else
		{
			m_iRetries++;

			if (m_iRetries > MAX_RETRIES)
				m_iTotalPackets = 0;

			return false;
		}

		if (curPacket == 0)
			offset = 4;

		if (curPacket < MAX_PACKETS)
		{
			m_CurPacket[curPacket].Clear();
			m_CurPacket[curPacket].WriteBuf(bytes - offset - sizeof(SPLITPACKET), &buffer[offset + sizeof(SPLITPACKET)]);
		}

		if (m_iCurrentPackets == m_iTotalPackets)
		{
			EnterCriticalSection((CRITICAL_SECTION *)m_pBufferCS);

			netadr_t addr;
			net->SockAddrToNetAdr(&from, &addr);

			int idx = m_MsgBuffers.AddToTail();
			CMsgBuffer &msgBuffer = m_MsgBuffers[idx];

			msgBuffer.Clear();

			for (int i = 0; i < m_iTotalPackets; i++)
			{
				m_CurPacket[i].ReadBuf(m_CurPacket[i].GetCurSize(), buffer);
				msgBuffer.WriteBuf(m_CurPacket[i].GetCurSize(), buffer);
			}

			msgBuffer.SetTime(recvTime);
			msgBuffer.SetNetAddress(addr);

			LeaveCriticalSection((CRITICAL_SECTION *)m_pBufferCS);

			m_iTotalPackets = 0;
		}
	}
	else if (*(int *)&buffer[0] == -1)
	{
		EnterCriticalSection((CRITICAL_SECTION *)m_pBufferCS);

		int idx = m_MsgBuffers.AddToTail();
		CMsgBuffer &msgBuffer = m_MsgBuffers[idx];

		msgBuffer.Clear();
		msgBuffer.WriteBuf(bytes - 4, &buffer[4]);
		msgBuffer.SetTime(recvTime);
		msgBuffer.SetNetAddress(addr);

		LeaveCriticalSection((CRITICAL_SECTION *)m_pBufferCS);
	}

	return true;
}