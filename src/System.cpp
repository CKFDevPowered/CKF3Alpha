#pragma comment(lib, "Shlwapi.lib")

#if !defined (_X360)
#define WIN32_LEAN_AND_MEAN
#define OEMRESOURCE
#include <windows.h>
#include <shellapi.h>
#include <shlwapi.h>
#include <shlobj.h>
#endif
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/stat.h>

#ifdef ShellExecute
#undef ShellExecute
#endif

#ifdef GetCurrentTime
#undef GetCurrentTime
#endif

#ifdef GetTickCount
#undef GetTickCount
#endif

#include <interface.h>
#include <vgui/VGUI.h>
#include <vgui/ISystem.h>
#include <KeyValues.h>
#include <vgui/IInputInternal.h>
#include <vgui/ISurface.h>
#include "tier0/vcrmode.h"
#include "FileSystem.h"

#include "vgui_internal.h"
#include "filesystem_helpers.h"
#include "vgui_key_translation.h"
#include "filesystem.h"

#if defined (_X360)
#include "xbox/xbox_win32stubs.h"
#endif

#define PROTECTED_THINGS_DISABLE

#include <tier0/memdbgon.h>

#ifndef _X360
static BOOL CALLBACK GetMainApplicationWindowHWND_EnumProc(HWND hWnd, LPARAM lParam)
{
	DWORD dwProcessId, dwThreadId;
	dwThreadId = GetWindowThreadProcessId(hWnd, &dwProcessId);

	DWORD dwOurProcessId;
	dwOurProcessId = GetCurrentProcessId();

	if (dwOurProcessId != dwProcessId)
		return TRUE;

	if (!IsWindowVisible(hWnd) || !IsWindowEnabled(hWnd))
		return TRUE;

	*(HWND *)lParam = hWnd;
	return FALSE;
}

static HWND GetMainApplicationWindowHWND(void)
{
	HWND hWnd = NULL;
	DWORD dwThreadId = GetCurrentThreadId();

	GUITHREADINFO gti;
	memset(&gti, 0, sizeof(gti));
	gti.cbSize = sizeof(gti);
	GetGUIThreadInfo(dwThreadId, &gti);

	hWnd = gti.hwndActive;

	for (HWND hParent = hWnd ? GetParent(hWnd) : hWnd; hParent; hWnd = hParent, hParent = GetParent(hWnd))
		continue;

	if (hWnd)
		return hWnd;

	EnumWindows(GetMainApplicationWindowHWND_EnumProc, (LPARAM) &hWnd);

	if (hWnd)
		return hWnd;

	return NULL;
}
#endif

using namespace vgui;

SHORT System_GetKeyState(int virtualKeyCode)
{
#ifndef _X360
	return GetKeyState(virtualKeyCode);
#else
	return 0;
#endif
}

class CSystem : public ISystem
{
public:
	CSystem(void);
	~CSystem(void);

public:
	virtual void Shutdown(void);
	virtual void RunFrame(void);

	virtual long GetTimeMillis(void);
	virtual double GetFrameTime(void);
	virtual double GetCurrentTime(void);

	virtual void ShellExecute(const char *command, const char *file);

	virtual int GetClipboardTextCount(void);
	virtual void SetClipboardText(const char *text, int textLen);
	virtual void SetClipboardText(const wchar_t *text, int textLen);
	virtual int GetClipboardText(int offset, char *buf, int bufLen);
	virtual int GetClipboardText(int offset, wchar_t *buf, int bufLen);

	virtual void SetClipboardImage(void *pWnd, int x1, int y1, int x2, int y2);

	virtual bool SetRegistryString(const char *key, const char *value);
	virtual bool GetRegistryString(const char *key, char *value, int valueLen);
	virtual bool SetRegistryInteger(const char *key, int value);
	virtual bool GetRegistryInteger(const char *key, int &value);
	virtual bool DeleteRegistryKey(const char *keyName);

	virtual bool SetWatchForComputerUse(bool state);
	virtual double GetTimeSinceLastUse(void);
	virtual int GetAvailableDrives(char *buf, int bufLen);
	virtual double GetFreeDiskSpace(const char *path);

	virtual KeyValues *GetUserConfigFileData(const char *dialogName, int dialogID);
	virtual void SetUserConfigFile(const char *fileName, const char *pathName);
	virtual void SaveUserConfigFile(void);

	virtual bool CommandLineParamExists(const char *commandName);
	virtual bool GetCommandLineParamValue(const char *paramName, char *value, int valueBufferSize);
	virtual const char *GetFullCommandLine(void);
	virtual bool GetCurrentTimeAndDate(int *year, int *month, int *dayOfWeek, int *day, int *hour, int *minute, int *second);

	virtual bool CreateShortcut(const char *linkFileName, const char *targetPath, const char *arguments, const char *workingDirectory, const char *iconFile);
	virtual bool GetShortcutTarget(const char *linkFileName, char *targetPath, char *arguments, int destBufferSizes);
	virtual bool ModifyShortcutTarget(const char *linkFileName, const char *targetPath, const char *arguments, const char *workingDirectory);

	virtual KeyCode KeyCode_VirtualKeyToVGUI(int keyCode);
	virtual const char *GetDesktopFolderPath(void);
	virtual void ShellExecuteEx(const char *command, const char *file, const char *pParams);

private:
	bool m_bStaticWatchForComputerUse;
	HHOOK m_hStaticKeyboardHook;
	HHOOK m_hStaticMouseHook;
	double m_StaticLastComputerUseTime;
	int m_iStaticMouseOldX, m_iStaticMouseOldY;

	double m_flFrameTime;
	KeyValues *m_pUserConfigData;
	char m_szFileName[MAX_PATH];
	char m_szPathID[MAX_PATH];
};

CSystem g_System;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CSystem, ISystem, VGUI_SYSTEM_INTERFACE_VERSION, g_System);

namespace vgui
{
vgui::ISystem *g_pSystem = &g_System;
}

CSystem::CSystem(void)
{
	m_bStaticWatchForComputerUse = false;
	m_hStaticKeyboardHook = NULL;
	m_hStaticMouseHook = NULL;
	m_StaticLastComputerUseTime = 0.0;
	m_iStaticMouseOldX = m_iStaticMouseOldY = -1;
	m_flFrameTime = 0.0;
	m_pUserConfigData = NULL;
}

CSystem::~CSystem(void)
{
}

void CSystem::Shutdown(void)
{
	if (m_pUserConfigData)
	{
		m_pUserConfigData->deleteThis();
		m_pUserConfigData = NULL;
	}
}

void CSystem::RunFrame(void)
{
	m_flFrameTime = GetCurrentTime();

	if (m_bStaticWatchForComputerUse)
	{
		int x, y;
		g_pInput->GetCursorPos(x, y);

		if (abs((x + y) - (m_iStaticMouseOldX + m_iStaticMouseOldY)) > 50)
		{
			m_StaticLastComputerUseTime = GetTimeMillis() / 1000.0;
			m_iStaticMouseOldX = x;
			m_iStaticMouseOldY = y;
		}
	}
}

double CSystem::GetFrameTime(void)
{
	return m_flFrameTime;
}

double CSystem::GetCurrentTime(void)
{
	return Plat_FloatTime();
}

long CSystem::GetTimeMillis(void)
{
	return (long)(GetCurrentTime() * 1000);
}

void CSystem::ShellExecute(const char *command, const char *file)
{
#ifndef _X360
	::ShellExecuteA(NULL, command, file, NULL, NULL, SW_SHOWNORMAL);
#endif
}

void CSystem::ShellExecuteEx(const char *command, const char *file, const char *pParams)
{
#ifndef _X360
	::ShellExecuteA(NULL, command, file, pParams, NULL, SW_SHOWNORMAL);
#endif
}

void CSystem::SetClipboardImage(void *pWnd, int x1, int y1, int x2, int y2)
{
#ifndef _X360
	if (x2 <= x1 || y2 <= y1)
		return;

	HWND hWnd = (HWND)(pWnd);

	if (!hWnd)
		hWnd = GetMainApplicationWindowHWND();

	if (!hWnd)
		return;

	HBITMAP hBmMem = NULL;
	{
		HDC hDc = GetDC(hWnd);
		HDC hDcMem = CreateCompatibleDC(hDc);
		hBmMem = CreateCompatibleBitmap(hDc, x2 - x1, y2 - y1);
		HBITMAP hBmOld = (HBITMAP)SelectObject(hDcMem, hBmMem);
		BitBlt(hDcMem, 0, 0, x2 - x1, y2 - y1, hDc, x1, y1, SRCCOPY);
		SelectObject(hDcMem, hBmOld);
		DeleteDC(hDcMem);
		ReleaseDC(hWnd, hDc);
	}

	if (!OpenClipboard(GetDesktopWindow()))
		return;

	EmptyClipboard();

	if (hBmMem)
	{
		SetClipboardData(CF_BITMAP, hBmMem);
	}

	CloseClipboard();
#endif
}

void CSystem::SetClipboardText(const char *text, int textLen)
{
#ifndef _X360
	if (!text)
		return;

	if (textLen <= 0)
		return;

	if (!OpenClipboard(GetDesktopWindow()))
		return;

	EmptyClipboard();

	HANDLE hmem = GlobalAlloc(GMEM_MOVEABLE, textLen + 1);

	if (hmem)
	{
		void *ptr = GlobalLock(hmem);

		if (ptr != null)
		{
			memset(ptr, 0, textLen + 1);
			memcpy(ptr, text, textLen);
			GlobalUnlock(hmem);

			SetClipboardData(CF_TEXT, hmem);
		}
	}
	
	CloseClipboard();
#endif
}

void CSystem::SetClipboardText(const wchar_t *text, int textLen)
{
#ifndef _X360
	if (!text)
		return;

	if (textLen <= 0)
		return;

	BOOL cb = OpenClipboard(GetDesktopWindow());

	if (!cb)
		return;

	EmptyClipboard();

	HANDLE hmem = GlobalAlloc(GMEM_MOVEABLE, (textLen + 1) * sizeof(wchar_t));

	if (hmem)
	{
		void *ptr = GlobalLock(hmem);

		if (ptr != null)
		{
			memset(ptr, 0, (textLen + 1) * sizeof(wchar_t));
			memcpy(ptr, text, textLen * sizeof(wchar_t));
			GlobalUnlock(hmem);

			SetClipboardData(CF_UNICODETEXT, hmem);
		}
	}

	CloseClipboard();
#endif
}

int CSystem::GetClipboardTextCount(void)
{
#ifndef _X360
	int count = 0;

	if (OpenClipboard(GetDesktopWindow()))
	{
		HANDLE hmem = GetClipboardData(CF_TEXT);

		if (hmem)
		{
			count = GlobalSize(hmem);
		}

		CloseClipboard();
	}

	return count;
#else
	return 0;
#endif
}

int CSystem::GetClipboardText(int offset, char *buf, int bufLen)
{
#ifndef _X360
	int count = 0;

	if (buf && bufLen > 0)
	{
		if (OpenClipboard(GetDesktopWindow()))
		{
			HANDLE hmem = GetClipboardData(CF_UNICODETEXT);

			if (hmem)
			{
				int len = GlobalSize(hmem);
				count = len - offset;

				if (count <= 0)
				{
					count = 0;
				}
				else
				{
					if (bufLen < count)
					{
						count = bufLen;
					}

					void *ptr = GlobalLock(hmem);

					if (ptr)
					{
						memcpy(buf, ((char *)ptr) + offset, count);
						GlobalUnlock(hmem);
					}
				}
			}

			CloseClipboard();
		}
	}

	return count;
#else
	return 0;
#endif
}

int CSystem::GetClipboardText(int offset, wchar_t *buf, int bufLen)
{
#ifndef _X360
	int retVal = 0;

	if (buf && bufLen > 0)
	{
		if (OpenClipboard(GetDesktopWindow()))
		{
			HANDLE hmem = GetClipboardData(CF_UNICODETEXT);

			if (hmem)
			{
				int len = GlobalSize(hmem);
				int count = len - offset;

				if (count > 0)
				{
					if (bufLen < count)
					{
						count = bufLen;
					}

					void *ptr = GlobalLock(hmem);

					if (ptr)
					{
						memcpy(buf, ((wchar_t *)ptr) + offset, count);
						retVal = count / sizeof(wchar_t);
						GlobalUnlock(hmem);
					}
				}
			}
		}

		CloseClipboard();
	}

	return retVal;
#else
	return 0;
#endif
}

static bool staticSplitRegistryKey(const char *key, char *key0, int key0Len, char *key1, int key1Len)
{
	if (key == null)
	{
		return false;
	}

	int len = strlen(key);

	if (len <= 0)
	{
		return false;
	}

	int Start = -1;

	for (int i = len - 1; i >= 0; i--)
	{
		if (key[i] == '\\')
		{
			break;
		}
		else
		{
			Start=i;
		}
	}

	if (Start == -1)
	{
		return false;
	}

	vgui_strcpy(key0, Start + 1, key);
	vgui_strcpy(key1, (len-Start) + 1, key + Start);
	return true;
}

bool CSystem::SetRegistryString(const char *key, const char *value)
{
#ifndef _X360
	HKEY hKey;
	HKEY hSlot = HKEY_CURRENT_USER;

	if (!strncmp(key, "HKEY_LOCAL_MACHINE", 18))
	{
		hSlot = HKEY_LOCAL_MACHINE;
		key += 19;
	}
	else if (!strncmp(key, "HKEY_CURRENT_USER", 17))
	{
		hSlot = HKEY_CURRENT_USER;
		key += 18;
	}

	char key0[256], key1[256];

	if (!staticSplitRegistryKey(key, key0, sizeof(key0), key1, sizeof(key1)))
	{
		return false;
	}

	if (RegCreateKeyEx(hSlot, key0, null, null, REG_OPTION_NON_VOLATILE, value ? KEY_WRITE : KEY_ALL_ACCESS, null, &hKey, null) != ERROR_SUCCESS)
	{
		return false;
	}

	if (RegSetValueEx(hKey, key1, NULL, REG_SZ, (uchar *)value, strlen(value) + 1) == ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return true;
	}

	RegCloseKey(hKey);
#endif
	return false;
}

bool CSystem::GetRegistryString(const char *key, char *value, int valueLen)
{
#ifndef _X360
	if (!value)
		return false;

	value[0] = 0;

	HKEY hKey;
	HKEY hSlot = HKEY_CURRENT_USER;

	if (!strncmp(key, "HKEY_LOCAL_MACHINE", 18))
	{
		hSlot = HKEY_LOCAL_MACHINE;
		key += 19;
	}
	else if (!strncmp(key, "HKEY_CURRENT_USER", 17))
	{
		hSlot = HKEY_CURRENT_USER;
		key += 18;
	}

	char key0[256], key1[256];

	if (!staticSplitRegistryKey(key, key0, 256, key1, 256))
	{
		return false;
	}

	if (RegOpenKeyEx(hSlot, key0, null, KEY_READ, &hKey) != ERROR_SUCCESS)
	{
		return false;
	}

	ulong len = valueLen;

	if (RegQueryValueEx(hKey, key1, null, null, (uchar *)value, &len) == ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return true;
	}

	RegCloseKey(hKey);
#endif
	return false;
}

bool CSystem::SetRegistryInteger(const char *key, int value)
{
#ifndef _X360
	HKEY hKey;
	HKEY hSlot = HKEY_CURRENT_USER;

	if (!strncmp(key, "HKEY_LOCAL_MACHINE", 18))
	{
		hSlot = HKEY_LOCAL_MACHINE;
		key += 19;
	}
	else if (!strncmp(key, "HKEY_CURRENT_USER", 17))
	{
		hSlot = HKEY_CURRENT_USER;
		key += 18;
	}

	char key0[256], key1[256];

	if (!staticSplitRegistryKey(key, key0, 256, key1, 256))
	{
		return false;
	}

	if (RegCreateKeyEx(hSlot, key0, null, null, REG_OPTION_NON_VOLATILE, KEY_WRITE, null, &hKey, null) != ERROR_SUCCESS)
	{
		return false;
	}

	if (RegSetValueEx(hKey,key1,null,REG_DWORD,(uchar*)&value,4)==ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return true;
	}

	RegCloseKey(hKey);
#endif
	return false;
}

bool CSystem::GetRegistryInteger(const char *key, int &value)
{
#ifndef _X360
	HKEY hKey;
	HKEY hSlot = HKEY_CURRENT_USER;

	if (!strncmp(key, "HKEY_LOCAL_MACHINE", 18))
	{
		hSlot = HKEY_LOCAL_MACHINE;
		key += 19;
	}
	else if (!strncmp(key, "HKEY_CURRENT_USER", 17))
	{
		hSlot = HKEY_CURRENT_USER;
		key += 18;
	}

	char key0[256], key1[256];

	if (!staticSplitRegistryKey(key, key0, 256, key1, 256))
	{
		return false;
	}

	if (RegOpenKeyEx(hSlot, key0, null, KEY_READ, &hKey) != ERROR_SUCCESS)
	{
		return false;
	}

	ulong len = 4;

	if (RegQueryValueEx(hKey, key1, null, null, (uchar *)&value, &len) == ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return true;
	}

	RegCloseKey(hKey);
#endif
	return false;
}

bool CSystem::DeleteRegistryKey(const char *key)
{
#ifndef _X360
	HKEY hSlot = HKEY_CURRENT_USER;

	if (!strncmp(key, "HKEY_LOCAL_MACHINE", 18))
	{
		hSlot = HKEY_LOCAL_MACHINE;
		key += 19;
	}
	else if (!strncmp(key, "HKEY_CURRENT_USER", 17))
	{
		hSlot = HKEY_CURRENT_USER;
		key += 18;
	}

	if (SHDeleteKey(hSlot, key) == ERROR_SUCCESS)
	{
		return true;
	}
#endif
	return false;
}

bool CSystem::SetWatchForComputerUse(bool state)
{
	if (state == m_bStaticWatchForComputerUse)
		return true;

	m_bStaticWatchForComputerUse = state;

	if (m_bStaticWatchForComputerUse)
	{
	}
	else
	{
	}
	
	return true;
}

double CSystem::GetTimeSinceLastUse(void)
{
	if (m_bStaticWatchForComputerUse)
	{
		return (GetTimeMillis() / 1000.0) - m_StaticLastComputerUseTime;
	}

	return 0.0f;
}

int CSystem::GetAvailableDrives(char *buf, int bufLen)
{
#ifndef _X360
	return GetLogicalDriveStrings(bufLen, buf);
#else
	return 0;
#endif
}

double CSystem::GetFreeDiskSpace(const char *path)
{
	char buf[_MAX_PATH];
	strcpy(buf, path);

	char *slash = strstr(buf, "\\");

	if (slash)
	{
		slash[1] = 0;
	}

	ULARGE_INTEGER userFreeBytes, totalBytes, totalFreeBytes;

	if (::GetDiskFreeSpaceEx(buf, &userFreeBytes, &totalBytes, &totalFreeBytes))
	{
		return (double)userFreeBytes.QuadPart;
	}

	return 0.0;
}

KeyValues *CSystem::GetUserConfigFileData(const char *dialogName, int dialogID)
{
	if (!m_pUserConfigData)
		return NULL;

	Assert(dialogName && *dialogName);

	if (dialogID)
	{
		char buf[256];
		Q_snprintf(buf, sizeof(buf), "%s_%d", dialogName, dialogID);
		dialogName = buf;
	}

	return m_pUserConfigData->FindKey(dialogName, true);
}

void CSystem::SetUserConfigFile(const char *fileName, const char *pathName)
{
	if (!m_pUserConfigData)
	{
		m_pUserConfigData = new KeyValues("UserConfigData");
	}

	strncpy(m_szFileName, fileName, sizeof(m_szFileName) - 1);
	strncpy(m_szPathID, pathName, sizeof(m_szPathID) - 1);

	m_pUserConfigData->UsesEscapeSequences(true);
	m_pUserConfigData->LoadFromFile(g_pFullFileSystem, m_szFileName, m_szPathID);
}

void CSystem::SaveUserConfigFile()
{
	if (m_pUserConfigData)
	{
		m_pUserConfigData->SaveToFile(g_pFullFileSystem, m_szFileName, m_szPathID);
	}
}

bool CSystem::CommandLineParamExists(const char *paramName)
{
	const char *cmdLine = GetFullCommandLine();
	const char *loc = strstr(cmdLine, paramName);
	return (loc != NULL);
}

bool CSystem::GetCommandLineParamValue(const char *paramName, char *value, int valueBufferSize)
{
	const char *cmdLine = GetFullCommandLine();
	const char *loc = strstr(cmdLine, paramName);

	if (!loc)
		return false;

	loc += strlen(paramName);

	char token[512];
	ParseFile(loc, token, NULL);

	strncpy(value, token, valueBufferSize - 1);
	value[valueBufferSize - 1] = 0;
	return true;
}

const char *CSystem::GetFullCommandLine(void)
{
	return GetCommandLine();
}

KeyCode CSystem::KeyCode_VirtualKeyToVGUI(int keyCode)
{
	return ::KeyCode_VirtualKeyToVGUI(keyCode);
}

bool CSystem::GetCurrentTimeAndDate(int *year, int *month, int *dayOfWeek, int *day, int *hour, int *minute, int *second)
{
	SYSTEMTIME time;
	GetLocalTime(&time);

	if (year)
	{
		*year = time.wYear;
	}

	if (month)
	{
		*month = time.wMonth;
	}

	if (dayOfWeek)
	{
		*dayOfWeek = time.wDayOfWeek;
	}

	if (day)
	{
		*day = time.wDay;
	}

	if (hour)
	{
		*hour = time.wHour;
	}

	if (minute)
	{
		*minute = time.wMinute;
	}

	if (second)
	{
		*second = time.wSecond;
	}

	return true;
}

bool CSystem::CreateShortcut(const char *linkFileName, const char *targetPath, const char *arguments, const char *workingDirectory, const char *iconFile)
{
#ifndef _X360
	bool bSuccess = false;
	char temp[MAX_PATH];
	strcpy(temp, linkFileName);

	struct _stat statBuf;

	if (_stat(linkFileName, &statBuf) != -1)
		return false;

	IShellLink *psl;
	HRESULT hres = ::CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *) &psl);

	if (SUCCEEDED(hres))
	{
		psl->SetPath(targetPath);
		psl->SetArguments(arguments);

		if (workingDirectory && *workingDirectory)
		{
			psl->SetWorkingDirectory(workingDirectory);
		}

		if (iconFile && *iconFile)
		{
			psl->SetIconLocation(iconFile, 0);
		}

		IPersistFile *ppf;
		hres = psl->QueryInterface(IID_IPersistFile, (LPVOID *)&ppf);

		if (SUCCEEDED(hres))
		{
			wchar_t wsz[MAX_PATH];
			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, temp, -1, wsz, MAX_PATH);
			hres = ppf->Save(wsz, TRUE);

			if (SUCCEEDED(hres))
			{
				bSuccess = true;
			}

			ppf->Release();
		}

		psl->Release();
	}

	return bSuccess;
#else
	return false;
#endif
}

bool CSystem::GetShortcutTarget(const char *linkFileName, char *targetPath, char *arguments, int destBufferSizes)
{
#ifndef _X360
	char temp[MAX_PATH];
	strcpy(temp, linkFileName);
	strlwr(temp);

	targetPath[0] = 0;
	arguments[0] = 0;

	IShellLink *psl;
	HRESULT hres = ::CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&psl);

	if (SUCCEEDED(hres))
	{
		IPersistFile *ppf;
		hres = psl->QueryInterface(IID_IPersistFile, (LPVOID *)&ppf);

		if (SUCCEEDED(hres))
		{
			wchar_t wsz[MAX_PATH];
			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, temp, -1, wsz, MAX_PATH);
			hres = ppf->Load(wsz, 0);

			if (SUCCEEDED(hres))
			{
				psl->GetPath(targetPath, destBufferSizes, NULL, SLGP_UNCPRIORITY);
				psl->GetArguments(arguments, destBufferSizes);
			}

			ppf->Release();
		}

		psl->Release();
	}

	return (targetPath[0] != 0);
#else
	return false;
#endif
}

bool CSystem::ModifyShortcutTarget(const char *linkFileName, const char *targetPath, const char *arguments, const char *workingDirectory)
{
#ifndef _X360
	bool bSuccess = false;
	char temp[MAX_PATH];
	strcpy(temp, linkFileName);
	strlwr(temp);

	IShellLink *psl;
	HRESULT hres = ::CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&psl);

	if (SUCCEEDED(hres))
	{
		IPersistFile *ppf;
		hres = psl->QueryInterface(IID_IPersistFile, (LPVOID *)&ppf);

		if (SUCCEEDED(hres))
		{
			wchar_t wsz[MAX_PATH];
			MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, temp, -1, wsz, MAX_PATH);
			hres = ppf->Load(wsz, 0);
			
			if (SUCCEEDED(hres))
			{ 
				psl->SetPath(targetPath);
				psl->SetArguments(arguments);
				psl->SetWorkingDirectory(workingDirectory);
				bSuccess = true;
				ppf->Save(wsz, TRUE);
			}

			ppf->Release();
		}

		psl->Release();
	}
#else
	return false;
#endif
	return bSuccess;
}

const char *CSystem::GetDesktopFolderPath(void)
{
#ifndef _X360
	static char folderPath[MAX_PATH];
	folderPath[0] = 0;

	if (GetRegistryString("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders\\Desktop", folderPath, sizeof(folderPath)) && strlen(folderPath) > 6)
	{
		return folderPath;
	}

	if (::SHGetSpecialFolderPath(NULL, folderPath, CSIDL_DESKTOP, false) && strlen(folderPath) > 6)
	{
		return folderPath;
	}
#endif
	return NULL;
}
