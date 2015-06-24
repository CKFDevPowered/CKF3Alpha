#include <metahook.h>
#include <time.h>
#include <eh.h>
#include "minidump.h"
#include "tier0/dbg.h"

static bool g_bWritingNonfatalMinidump = false;
static int g_nMinidumpsWritten = 0;
static DWORD g_dwLastMiniDumpFileSize = 0;

DWORD GetLastMiniDumpFileSize(void)
{
	return g_dwLastMiniDumpFileSize;
}

bool WriteMiniDumpUsingExceptionInfo(unsigned int uStructuredExceptionCode, _EXCEPTION_POINTERS *pExceptionInfo, MINIDUMP_TYPE minidumpType, tchar *ptchMinidumpFileNameBuffer)
{
	typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType, CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

	if (ptchMinidumpFileNameBuffer)
		*ptchMinidumpFileNameBuffer = tchar(0);

	HMODULE hDbgHelpDll = LoadLibrary("DbgHelp.dll");

	if (!hDbgHelpDll)
		return false;

	bool bReturnValue = false;
	MINIDUMPWRITEDUMP pfnMiniDumpWrite = (MINIDUMPWRITEDUMP)GetProcAddress(hDbgHelpDll, "MiniDumpWriteDump");

	if (pfnMiniDumpWrite)
	{
		time_t currTime = time(NULL);
		struct tm * pTime = localtime(&currTime);
		++g_nMinidumpsWritten;

		tchar rgchModuleName[MAX_PATH];
		GetModuleFileName(NULL, rgchModuleName, sizeof(rgchModuleName) / sizeof(tchar));

		tchar *pch = _tcsrchr(rgchModuleName, '.');

		if (pch)
			*pch = 0;

		pch = _tcsrchr(rgchModuleName, '\\');

		if (pch)
			pch++;
		else
			pch = _T("unknown");

		tchar rgchFileName[MAX_PATH];
		_sntprintf(rgchFileName, sizeof(rgchFileName) / sizeof(tchar), _T("%s_%s_%d%.2d%2d%.2d%.2d%.2d_%d.mdmp"), pch, "crash", pTime->tm_year + 1900,	pTime->tm_mon + 1, pTime->tm_mday, pTime->tm_hour, pTime->tm_min, pTime->tm_sec, g_nMinidumpsWritten);

		BOOL bMinidumpResult = FALSE;
		HANDLE hFile = CreateFile(rgchFileName, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFile)
		{
			_MINIDUMP_EXCEPTION_INFORMATION	ExInfo;
			ExInfo.ThreadId = GetCurrentThreadId();
			ExInfo.ExceptionPointers = pExceptionInfo;
			ExInfo.ClientPointers = FALSE;

			bMinidumpResult = pfnMiniDumpWrite(GetCurrentProcess(), GetCurrentProcessId(), hFile, minidumpType, &ExInfo, NULL, NULL);
			g_dwLastMiniDumpFileSize = GetFileSize(hFile, NULL);

			CloseHandle(hFile);

			if (bMinidumpResult)
			{
				bReturnValue = true;

				if (ptchMinidumpFileNameBuffer)
				{
					tchar *pTgt = ptchMinidumpFileNameBuffer;
					tchar const *pSrc = rgchFileName;

					while ((*(pTgt++) = *(pSrc++)) != tchar(0))
						continue;
				}
			}
		}

		if (!bMinidumpResult)
		{
			tchar rgchFailedFileName[_MAX_PATH];
			_sntprintf(rgchFailedFileName, sizeof(rgchFailedFileName) / sizeof(tchar), "(failed)%s", rgchFileName);
			rename(rgchFileName, rgchFailedFileName);
		}
	}

	FreeLibrary(hDbgHelpDll);
	return bReturnValue;
}

void InternalWriteMiniDumpUsingExceptionInfo(unsigned int uStructuredExceptionCode, _EXCEPTION_POINTERS *pExceptionInfo)
{
	int iType = MiniDumpWithDataSegs | MiniDumpWithIndirectlyReferencedMemory;

	if (!WriteMiniDumpUsingExceptionInfo(uStructuredExceptionCode, pExceptionInfo, (MINIDUMP_TYPE)iType))
	{
		iType = MiniDumpWithDataSegs;
		WriteMiniDumpUsingExceptionInfo(uStructuredExceptionCode, pExceptionInfo, (MINIDUMP_TYPE)iType);
	}
}

static FnMiniDump g_pfnWriteMiniDump = InternalWriteMiniDumpUsingExceptionInfo;

LONG WINAPI InternalExceptionHandle(unsigned int uStructuredExceptionCode, struct _EXCEPTION_POINTERS *pExceptionInfo)
{
	g_pfnWriteMiniDump(uStructuredExceptionCode, pExceptionInfo);
	return EXCEPTION_EXECUTE_HANDLER;
}

FnMiniDump SetMiniDumpFunction(FnMiniDump pfn)
{
	FnMiniDump pfnTemp = g_pfnWriteMiniDump;
	g_pfnWriteMiniDump = pfn;
	return pfnTemp;
}

void WriteMiniDump(void)
{
	g_bWritingNonfatalMinidump = true;

	__try
	{
		::RaiseException(0, EXCEPTION_NONCONTINUABLE, 0, NULL);
	}

	__except (g_pfnWriteMiniDump(0, GetExceptionInformation()), EXCEPTION_EXECUTE_HANDLER)
	{
	}

	g_bWritingNonfatalMinidump = false;
}

DBG_OVERLOAD bool g_bInException = false;

void CatchAndWriteMiniDump(FnWMain pfn, int argc, tchar *argv[])
{
	if (Plat_IsInDebugSession())
	{
		pfn(argc, argv);
	}
	else
	{
		try
		{
			_set_se_translator(g_pfnWriteMiniDump);
			pfn(argc, argv);
		}
		catch (...)
		{
			g_bInException = true;
			DMsg("console", 1, _T("Fatal exception caught, minidump written\n"));
		}
	}
}

void SetupExceptionHandle(void)
{
	LONG (WINAPI *pfnSteamWriteMiniDumpUsingExceptionInfo)(unsigned int uStructuredExceptionCode, struct _EXCEPTION_POINTERS *pExceptionInfo) = (LONG (WINAPI *)(unsigned int, struct _EXCEPTION_POINTERS *))GetProcAddress(GetModuleHandle("Steam.dll"), "SteamWriteMiniDumpUsingExceptionInfo");

	if (pfnSteamWriteMiniDumpUsingExceptionInfo)
		g_pMetaHookAPI->InlineHook(pfnSteamWriteMiniDumpUsingExceptionInfo, InternalExceptionHandle, (void *&)pfnSteamWriteMiniDumpUsingExceptionInfo);
}