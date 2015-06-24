#include <windows.h>

BOOL Sys_CloseDEP(void)
{
	static BOOL (WINAPI *SetProcessDEPPolicy)(DWORD dwFlags) = (BOOL (WINAPI *)(DWORD dwFlags))GetProcAddress(GetModuleHandle("kernel32.dll"), "SetProcessDEPPolicy");

	if (SetProcessDEPPolicy)
	{
		if (SetProcessDEPPolicy(0))
			return TRUE;
	}

	static LONG (WINAPI *NtSetInformationProcess)(HANDLE ProcessHandle, UINT ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength) = (LONG (WINAPI *)(HANDLE, UINT, PVOID, ULONG))GetProcAddress(GetModuleHandle("ntdll.dll"), "NtSetInformationProcess");

	if (NtSetInformationProcess)
	{
		ULONG ExecuteFlags = 2;
		return (NtSetInformationProcess(GetCurrentProcess(), 34, &ExecuteFlags, sizeof(ExecuteFlags)) == 0);
	}

	return FALSE;
}

BOOL Sys_GetExecutableName(char *pszName, int nSize)
{
	return GetModuleFileName(GetModuleHandle(NULL), pszName, nSize) != 0;
}

char *Sys_GetLongPathName(void)
{
	char szShortPath[MAX_PATH];
	static char szLongPath[MAX_PATH];
	char *pszPath;

	szShortPath[0] = 0;
	szLongPath[0] = 0;

	if (GetModuleFileName(NULL, szShortPath, sizeof(szShortPath)))
	{
		GetLongPathName(szShortPath, szLongPath, sizeof(szLongPath));
		pszPath = strrchr(szLongPath, '\\');

		if (pszPath[0])
			pszPath[1] = 0;

		size_t len = strlen(szLongPath);

		if (len > 0)
		{
			if (szLongPath[len - 1] == '\\' || szLongPath[len - 1] == '/')
				szLongPath[len - 1] = 0;
		}
	}

	return szLongPath;
}