#define WIN_32_LEAN_AND_MEAN
#include <windows.h>
#include "hook.h"
#include <time.h>
#include <stdio.h>
#include <eh.h>
#include "minidump.h"

int g_nMinidumpsWritten = 0;
DWORD g_dwLastMiniDumpFileSize = 0;

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

static bool g_bInException = false;
static FnMiniDump g_pfnWriteMiniDump = InternalWriteMiniDumpUsingExceptionInfo;

void CatchAndWriteMiniDump(FnWMain pfn, int argc, tchar *argv[])
{
	try
	{
		_set_se_translator(g_pfnWriteMiniDump);
		pfn(argc, argv);
	}
	catch (...)
	{
		g_bInException = true;
	}
}

void CatchAndWriteMiniDumpForVoidPtrFn(FnVoidPtrFn pfn, void *pv, bool bExitQuietly)
{
	try
	{
		_set_se_translator(g_pfnWriteMiniDump);
		pfn(pv);
	}
	catch (...)
	{
		g_bInException = true;
	}
}

void CatchAndWriteMiniDumpEx(FnWMain pfn, int argc, tchar *argv[], ECatchAndWriteMinidumpAction eAction)
{
	try
	{
		_set_se_translator(g_pfnWriteMiniDump);
		pfn(argc, argv);
	}
	catch (...)
	{
		g_bInException = true;
	}
}

int CatchAndWriteMiniDumpExReturnsInt(FnWMainIntRet pfn, int argc, tchar *argv[], ECatchAndWriteMinidumpAction eAction)
{
	int ret = 0;

	try
	{
		_set_se_translator(g_pfnWriteMiniDump);
		ret = pfn(argc, argv);
	}
	catch (...)
	{
		g_bInException = true;
	}

	return ret;
}

void CatchAndWriteMiniDumpExForVoidPtrFn(FnVoidPtrFn pfn, void *pv, ECatchAndWriteMinidumpAction eAction)
{
	try
	{
		_set_se_translator(g_pfnWriteMiniDump);
		pfn(pv);
	}
	catch (...)
	{
		g_bInException = true;
	}
}