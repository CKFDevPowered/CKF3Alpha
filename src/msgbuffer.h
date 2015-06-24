#if !defined (MSGBUFFER_H)
#define MSGBUFFER_H

#ifdef _WIN32
#pragma once
#endif

#include "netadr.h"

class CMsgBuffer
{
public:
	enum
	{
		NET_MAXMESSAGE = 8192
	};

public:
	CMsgBuffer(const char *buffername = "unnamed", void (*ef)(const char *fmt, ...) = 0);
	virtual ~CMsgBuffer(void);

public:
	void Clear(void);
	size_t GetCurSize(void);
	size_t GetMaxSize(void);
	void *GetData(void);
	void SetOverflow(bool allowed);
	void BeginReading(void);
	size_t GetReadCount(void);

	void Push(void);
	void Pop(void);

	void WriteByte(int c);
	void WriteShort(int c);
	void WriteLong(int c);
	void WriteFloat(float f);
	void WriteString(const char *s);
	void WriteBuf(size_t iSize, void *buf);

	int ReadByte(void);
	int ReadShort(void);
	int ReadLong(void);
	float ReadFloat(void);
	char *ReadString(void);
	size_t ReadBuf(size_t iSize, void *pbuf);

	void SetTime(float time);
	float GetTime(void);

	void SetNetAddress(netadr_t &adr);
	netadr_t &GetNetAddress(void);

private:
	void *GetSpace(size_t length);
	void Write(const void *data, size_t length);

private:
	const char *m_pszBufferName;
	void (*m_pfnErrorFunc)(const char *fmt, ...);

	size_t m_nReadCount;
	size_t m_nPushedCount;
	bool m_bPushed;
	bool m_bBadRead;
	size_t m_nMaxSize;
	size_t m_nCurSize;
	bool m_bAllowOverflow;
	bool m_bOverFlowed;
	unsigned char m_rgData[NET_MAXMESSAGE];
	float m_fRecvTime;
	netadr_t m_NetAddr;
};

#endif