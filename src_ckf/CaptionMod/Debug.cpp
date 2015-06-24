#include <windows.h>
#include <tier0/dbg.h>

#pragma warning(push)
#pragma warning(disable:4273)

void Msg(const tchar *pMsg, ...)
{
#ifdef _DEBUG
	va_list argptr;
	static char buffer[1024];

	va_start(argptr, pMsg);
	vsprintf(buffer, pMsg, argptr);
	va_end(argptr);

	OutputDebugString(buffer);
#endif
}

void Warning(const tchar *pMsg, ...)
{
#ifdef _DEBUG
	va_list argptr;
	static char buffer[1024];

	va_start(argptr, pMsg);
	vsprintf(buffer, pMsg, argptr);
	va_end(argptr);

	OutputDebugString(buffer);
#endif
}

void Log(const tchar *pMsg, ...)
{
#ifdef _DEBUG
	va_list argptr;
	static char buffer[1024];

	va_start(argptr, pMsg);
	vsprintf(buffer, pMsg, argptr);
	va_end(argptr);

	OutputDebugString(buffer);
#endif
}

void Error(const tchar *pMsg, ...)
{
#ifdef _DEBUG
	va_list argptr;
	static char buffer[1024];

	va_start(argptr, pMsg);
	vsprintf(buffer, pMsg, argptr);
	va_end(argptr);

	OutputDebugString(buffer);
#endif
}

void DevMsg(int level, const tchar *pMsg, ...)
{
#ifdef _DEBUG
	va_list argptr;
	static char buffer[1024];

	va_start(argptr, pMsg);
	vsprintf(buffer, pMsg, argptr);
	va_end(argptr);

	OutputDebugString(buffer);
#endif
}

void DevWarning(int level, const tchar *pMsg, ...)
{
#ifdef _DEBUG
	va_list argptr;
	static char buffer[1024];

	va_start(argptr, pMsg);
	vsprintf(buffer, pMsg, argptr);
	va_end(argptr);

	OutputDebugString(buffer);
#endif
}

void DevLog(int level, const tchar *pMsg, ...)
{
#ifdef _DEBUG
	va_list argptr;
	static char buffer[1024];

	va_start(argptr, pMsg);
	vsprintf(buffer, pMsg, argptr);
	va_end(argptr);

	OutputDebugString(buffer);
#endif
}

void DevMsg(const tchar *pMsg, ...)
{
#ifdef _DEBUG
	va_list argptr;
	static char buffer[1024];

	va_start(argptr, pMsg);
	vsprintf(buffer, pMsg, argptr);
	va_end(argptr);

	OutputDebugString(buffer);
#endif
}

void DevWarning(const tchar *pMsg, ...)
{
#ifdef _DEBUG
	va_list argptr;
	static char buffer[1024];

	va_start(argptr, pMsg);
	vsprintf(buffer, pMsg, argptr);
	va_end(argptr);

	OutputDebugString(buffer);
#endif
}

void DevLog(const tchar *pMsg, ...)
{
#ifdef _DEBUG
	va_list argptr;
	static char buffer[1024];

	va_start(argptr, pMsg);
	vsprintf(buffer, pMsg, argptr);
	va_end(argptr);

	OutputDebugString(buffer);
#endif
}

#pragma warning(pop)