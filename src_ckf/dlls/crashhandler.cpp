#include <windows.h>
#include <DbgHelp.h>
#include <stdio.h>
#include "hook.h"
#include "minidump.h"
#include "perf_counter.h"

#pragma comment(lib, "Dbghelp.lib")

CPerformanceCounter gPerformanceCounter;

typedef bool (__stdcall *fnRtlDispatchException)(PEXCEPTION_RECORD pExcptRec, PCONTEXT pContext);
fnRtlDispatchException g_pfnRtlDispatchException;

bool WINAPI Hook_RtlDispatchException(PEXCEPTION_RECORD ExceptionRecord, PCONTEXT ContextRecord)
{
	bool status = g_pfnRtlDispatchException(ExceptionRecord, ContextRecord);

	if (!status)
	{
		EXCEPTION_POINTERS ExceptionInfo;
		MINIDUMP_EXCEPTION_INFORMATION eInfo;

		ExceptionInfo.ContextRecord = ContextRecord;
		ExceptionInfo.ExceptionRecord = ExceptionRecord;

		eInfo.ThreadId = GetCurrentThreadId();
		eInfo.ExceptionPointers = &ExceptionInfo;
		eInfo.ClientPointers = FALSE;

		SYSTEMTIME stCurTime;
		GetLocalTime(&stCurTime);

		HMODULE hModule = GetModuleHandle(NULL);

		char szModuleName[64];
		GetModuleFileName(hModule, szModuleName, sizeof(szModuleName));

		char *szExeName = strrchr(szModuleName, '\\') + 1;
		szExeName[strlen(szExeName) - 4] = 0;

		char szDate[128];
		sprintf(szDate, "%s_%04d_%d_%02d_%02d_%02d_%02d.mdmp", szExeName, stCurTime.wYear, stCurTime.wMonth, stCurTime.wDay, stCurTime.wHour, stCurTime.wMinute, stCurTime.wSecond);

		HANDLE hFile = CreateFile(szDate, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &eInfo, NULL, NULL);
		CloseHandle(hFile);
	}

	return status;
}

void InitExceptionFilter(void)
{
	DWORD oldProtect;
	PBYTE pFarProc = (PBYTE)GetProcAddress(LoadLibrary("ntdll.dll"), "KiUserExceptionDispatcher");

	for (DWORD i = 0; i < 40; i++)
	{
		if (*(pFarProc + i) == 0xE8)
		{
			g_pfnRtlDispatchException = (fnRtlDispatchException)((DWORD)(pFarProc + i) + *(DWORD *)(pFarProc + i + 1) + 5);
			VirtualProtect((PVOID)(pFarProc + i), 6, PAGE_EXECUTE_READWRITE, &oldProtect);
			*(DWORD *)(pFarProc + i + 1) = (DWORD)&Hook_RtlDispatchException - (DWORD)(pFarProc + i) - 5;
			break;
		}
	}
}

void CatchAndWriteMiniDump(FnWMain pfn, int argc, tchar *argv[]);
void CatchAndWriteMiniDumpForVoidPtrFn(FnVoidPtrFn pfn, void *pv, bool bExitQuietly);
void CatchAndWriteMiniDumpEx(FnWMain pfn, int argc, tchar *argv[], ECatchAndWriteMinidumpAction eAction);
int CatchAndWriteMiniDumpExReturnsInt(FnWMainIntRet pfn, int argc, tchar *argv[], ECatchAndWriteMinidumpAction eAction);
void CatchAndWriteMiniDumpExForVoidPtrFn(FnVoidPtrFn pfn, void *pv, ECatchAndWriteMinidumpAction eAction);

void (*g_pfnCatchAndWriteMiniDump)(FnWMain pfn, int argc, tchar *argv[]);
void (*g_pfnCatchAndWriteMiniDumpForVoidPtrFn)(FnVoidPtrFn pfn, void *pv, bool bExitQuietly);
void (*g_pfnCatchAndWriteMiniDumpEx)(FnWMain pfn, int argc, tchar *argv[], ECatchAndWriteMinidumpAction eAction);
int (*g_pfnCatchAndWriteMiniDumpExReturnsInt)(FnWMainIntRet pfn, int argc, tchar *argv[], ECatchAndWriteMinidumpAction eAction);
void (*g_pfnCatchAndWriteMiniDumpExForVoidPtrFn)(FnVoidPtrFn pfn, void *pv, ECatchAndWriteMinidumpAction eAction);

void InitCrashHandler(void)
{
	HMODULE tier0 = GetModuleHandle("tier0_s.dll");

	if (tier0)
	{
		DWORD dwCatchAndWriteMiniDump = (DWORD)GetProcAddress(tier0, "CatchAndWriteMiniDump");
		DWORD dwCatchAndWriteMiniDumpForVoidPtrFn = (DWORD)GetProcAddress(tier0, "CatchAndWriteMiniDumpForVoidPtrFn");
		DWORD dwCatchAndWriteMiniDumpEx = (DWORD)GetProcAddress(tier0, "CatchAndWriteMiniDumpEx");
		DWORD dwCatchAndWriteMiniDumpExReturnsInt = (DWORD)GetProcAddress(tier0, "CatchAndWriteMiniDumpExReturnsInt");
		DWORD dwCatchAndWriteMiniDumpExForVoidPtrFn = (DWORD)GetProcAddress(tier0, "CatchAndWriteMiniDumpExForVoidPtrFn");

		if (dwCatchAndWriteMiniDump)
			MH_InlineHook((void *)dwCatchAndWriteMiniDump, CatchAndWriteMiniDump, (void *&)g_pfnCatchAndWriteMiniDump);

		if (dwCatchAndWriteMiniDumpForVoidPtrFn)
			MH_InlineHook((void *)dwCatchAndWriteMiniDumpForVoidPtrFn, CatchAndWriteMiniDumpForVoidPtrFn, (void *&)g_pfnCatchAndWriteMiniDumpForVoidPtrFn);

		if (dwCatchAndWriteMiniDumpEx)
			MH_InlineHook((void *)dwCatchAndWriteMiniDumpEx, CatchAndWriteMiniDumpEx, (void *&)g_pfnCatchAndWriteMiniDumpEx);

		if (dwCatchAndWriteMiniDumpExReturnsInt)
			MH_InlineHook((void *)dwCatchAndWriteMiniDumpExReturnsInt, CatchAndWriteMiniDumpExReturnsInt, (void *&)g_pfnCatchAndWriteMiniDumpExReturnsInt);

		if (dwCatchAndWriteMiniDumpExForVoidPtrFn)
			MH_InlineHook((void *)dwCatchAndWriteMiniDumpExForVoidPtrFn, CatchAndWriteMiniDumpExForVoidPtrFn, (void *&)g_pfnCatchAndWriteMiniDumpExForVoidPtrFn);
	}
	else
	{
		//MessageBox(NULL, "tier0_s.dll not found", "Warning", MB_ICONWARNING);
	}
}

void InitPrefCounter(void)
{
	gPerformanceCounter.InitializePerformanceCounter();
}