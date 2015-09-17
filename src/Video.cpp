#define _WIN32_WINNT 0x0501
#include <metahook.h>

#include "cmd.h"
#include "cvar.h"
#include "console.h"
#include "hud.h"
#include "cl_util.h"
#include "Video.h"
#include "Plugins.h"
#include <keydefs.h>
#include <cvardef.h>
#include <math.h>
#include "VideoMode.h"
#include "vgui_internal.h"
#include <vgui/IInputInternal.h>

cvar_t vid_monitorgamma = { "vid_monitorgamma", "2.2", FCVAR_ARCHIVE };

HWND g_hMainWnd = NULL;
HDC g_hMainDC = NULL;
WNDPROC g_WndProc;

BOOL g_fActiveApp = FALSE;
BOOL g_fMinimized = FALSE;

VOID **g_ppvVideoBuffer = NULL;

//ATOM (WINAPI *g_pfnRegisterClassA)(WNDCLASSA *lpWndClass) = NULL;
HWND (WINAPI *g_pfnCreateWindowExA)(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam) = NULL;
BOOL (WINAPI *g_pfnDestroyWindow)(HWND hWnd) = NULL;
BOOL (WINAPI *g_pfnSetCursorPos)(int X, int Y) = NULL;
HBITMAP (WINAPI *g_pfnCreateDIBSection)(HDC hdc, CONST BITMAPINFO *lpbmi, UINT usage, VOID **ppvBits, HANDLE hSection, DWORD offset);

//hook_t *g_phRegisterClassA = NULL;
hook_t *g_phCreateWindowExA = NULL;
hook_t *g_phDestroyWindow = NULL;
hook_t *g_phSetCursorPos = NULL;
hook_t *g_phChangeDisplaySettingsA = NULL;
hook_t *g_phShowWindow = NULL;
hook_t *g_phCreateDIBSection = NULL;

extern int g_iVideoWidth, g_iVideoHeight, g_iBPP;
extern bool g_bWindowed;

static bool s_bSupportRelative = false;
static BOOL s_bSupportGamma = false;

static int s_iMouseDeltaX = 0, s_iMouseDeltaY = 0;
static int s_iMouseX = 0, s_iMouseY = 0;
static int s_iMouseLastX = 0, s_iMouseLastY = 0;
static int s_iMouseButton = 0;
static bool s_bRelativeMouse = false;
static bool s_bCorsorLock = false;
static bool s_bCursorHide = false;

static float s_flCurrentGamma = -1.0;
static unsigned short s_usGammaRamp[768];
static unsigned short s_usStateRamp[768];

LRESULT VID_MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

//
//ATOM WINAPI VID_RegisterClassA(WNDCLASSA *lpWndClass)
//{
//	return g_pfnRegisterClassA(lpWndClass);
//}

HWND WINAPI VID_CreateWindowExA(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam)
{
	BOOL bIsMainWnd = false;
	HWND hWnd = NULL;

	if (HIWORD(lpClassName))
	{
		if (!strcmp(lpClassName, "Valve001") || !strcmp(lpClassName, "SDL_app"))
			bIsMainWnd = true;
	}

	hWnd = g_pfnCreateWindowExA(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);

	if (bIsMainWnd && !g_hMainWnd)
	{
		VID_SetMainWindow(hWnd);
	}

	return hWnd;
}

BOOL WINAPI VID_DestroyWindow(HWND hWnd)
{
	if ( hWnd == g_hMainWnd )
		VID_RestoreGamma();

	return g_pfnDestroyWindow(hWnd);
}

BOOL WINAPI VID_SetCursorPos(int X, int Y)
{
	bool bCursorVisible = vgui::surface()->IsCursorVisible();
	//if (bCursorVisible && !(g_fActiveApp || g_fMinimized))
	//	return TRUE;
	if(bCursorVisible)
		return TRUE;

	if (X < 0 || Y < 0)
		return TRUE;

	return g_pfnSetCursorPos(X, Y);
}

BOOL VID_IsMinimized(void)
{
	return g_fMinimized;
}

void VID_BuildGamma(void)
{
	double invGamma = 1.0 / (3.1 - max(1.6, min(s_flCurrentGamma, 2.6)));
	double div = (double)1.0 / 255.5;

	for (int i = 0; i < 256; i++)
	{
		s_usGammaRamp[i + 0] = s_usGammaRamp[i + 256] = s_usGammaRamp[i + 512] = (unsigned short)(65535.0 * pow(((double)i + 0.5) * div, invGamma) + 0.5);
	}
}

void VID_CheckGamma(void)
{
	if (g_bWindowed)
		return;

	if (!s_bSupportGamma)
		return;

	if (s_flCurrentGamma != vid_monitorgamma.value)
	{
		VID_BuildGamma();

		if (SetDeviceGammaRamp(g_hMainDC, s_usGammaRamp))
			s_flCurrentGamma = vid_monitorgamma.value;
	}
}

void VID_RestoreGamma(void)
{
	if (g_bWindowed)
		return;

	if (!s_bSupportGamma)
		return;

	if (SetDeviceGammaRamp(g_hMainDC, s_usStateRamp))
		s_flCurrentGamma = -1;
}

bool VID_IsSupportsItem(int item)
{
	switch (item)
	{
		case VIDEOITEM_RELATIVEMOUSE:
		{
			return s_bSupportRelative;
		}

		case VIDEOITEM_MONITORGAMMA:
		{
			return s_bSupportGamma != false;
		}
	}

	return false;
}

void VID_UnlockCursor(void)
{
	ClipCursor(NULL);

	s_bCorsorLock = false;
}

void VID_LockCursor(void)
{
	if (!g_fActiveApp)
	{
		s_bCorsorLock = false;
		return;
	}
	
	LONG cx, cy;
	RECT rect;
	GetWindowRect(g_hMainWnd, &rect);

	if (s_bRelativeMouse)
	{
		cx = (rect.left + rect.right) / 2;
		cy = (rect.top + rect.bottom) / 2;

		rect.left = cx - 1;
		rect.right = cx + 1;
		rect.top = cy - 1;
		rect.bottom = cy + 1;
	}

	ClipCursor(&rect);
	s_bCorsorLock = true;
}

void VID_HideCursor(void)
{
	ShowCursor(FALSE);
	s_bCursorHide = true;
}

void VID_ShowCursor(void)
{
	ShowCursor(TRUE);
	s_bCursorHide = false;
}

void VID_SetRelativeMouseMode(bool enabled)
{
	s_bRelativeMouse = enabled;
}

BOOL VID_GetRelativeMouseState(int *x, int *y)
{
	if (x)
	{
		*x = s_iMouseDeltaX;
	}

	if (y)
	{
		*y = s_iMouseDeltaY;
	}

	s_iMouseDeltaX = 0;
	s_iMouseDeltaY = 0;

	return s_bSupportRelative;
}

int VID_GetRelativeMouseButton(void)
{
	int button = s_iMouseButton;

	s_iMouseButton = 0;
	return button;
}

void VID_MouseButton(int state)
{
	s_iMouseButton = 0;

	if (state & RI_MOUSE_BUTTON_1_DOWN)
		s_iMouseButton |= 1;

	if (state & RI_MOUSE_BUTTON_2_DOWN)
		s_iMouseButton |= 2;

	if (state & RI_MOUSE_BUTTON_3_DOWN)
		s_iMouseButton |= 2;
}

void VID_MouseMotion(int x, int y)
{
	int xrel = x;
	int yrel = y;

	x = (s_iMouseLastX + x);
	y = (s_iMouseLastY + y);

	s_iMouseX += xrel;
	s_iMouseY += yrel;

	s_iMouseDeltaX += xrel;
	s_iMouseDeltaY += yrel;

	s_iMouseLastX = x;
	s_iMouseLastY = y;
}

void VID_ClearMouseState(void)
{
	s_iMouseX = 0;
	s_iMouseY = 0;

	s_iMouseDeltaX = 0;
	s_iMouseDeltaY = 0;

	s_iMouseLastX = 0;
	s_iMouseLastY = 0;

	s_iMouseButton = 0;
}

static bool s_bIMEComposing = false;
static HWND s_hLastHWnd = 0;

void VID_SetMainWindow(HWND hWnd)
{
	g_hMainWnd = hWnd;
	g_hMainDC = GetDC(hWnd);
	g_WndProc = (WNDPROC)GetWindowLong(hWnd, GWL_WNDPROC);

	RAWINPUTDEVICE Rid;
	Rid.usUsagePage = 0x01;
	Rid.usUsage = 0x02;
	Rid.dwFlags = RIDEV_CAPTUREMOUSE;
	Rid.hwndTarget = hWnd;

	if (RegisterRawInputDevices(&Rid, 1, sizeof(Rid)) != FALSE)
		s_bSupportRelative = true;

	SetWindowLong(hWnd, GWL_WNDPROC, (LONG)VID_MainWndProc);	
}

LRESULT VID_MainWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (hWnd != s_hLastHWnd)
	{
		s_hLastHWnd = hWnd;
		vgui::g_pInput->SetIMEWindow(hWnd);
	}

	switch (uMsg)
	{
		case WM_CREATE:
		{
			break;
		}

		case WM_SYSCHAR:
		case WM_CHAR:
		{
			if (s_bIMEComposing)
				return 1;

			break;
		}

		case WM_INPUTLANGCHANGE:
		{
			vgui::g_pInput->OnInputLanguageChanged();
			break;
		}

		case WM_IME_STARTCOMPOSITION:
		{
			s_bIMEComposing = true;
			vgui::g_pInput->OnIMEStartComposition();
			return 1;
		}

		case WM_IME_COMPOSITION:
		{
			int flags = (int)lParam;

			vgui::g_pInput->OnIMEComposition(flags);
			return 1;
		}

		case WM_IME_ENDCOMPOSITION:
		{
			s_bIMEComposing = false;
			vgui::g_pInput->OnIMEEndComposition();
			return 1;
		}

		case WM_IME_NOTIFY:
		{
			switch (wParam)
			{
				default:
				{
					break;
				}

				case 14:
				{
					if (VID_IsWindowed())
						return 0;

					break;
				}

				case IMN_OPENCANDIDATE:
				{
					vgui::g_pInput->OnIMEShowCandidates();
					return 1;
				}

				case IMN_CHANGECANDIDATE:
				{
					vgui::g_pInput->OnIMEChangeCandidates();
					return 0;
				}

				case IMN_CLOSECANDIDATE:
				{
					vgui::g_pInput->OnIMECloseCandidates();
					break;
				}

				case IMN_SETCONVERSIONMODE:
				case IMN_SETSENTENCEMODE:
				case IMN_SETOPENSTATUS:   
				{
					vgui::g_pInput->OnIMERecomputeModes();
					break;
				}

				case IMN_CLOSESTATUSWINDOW:   
				case IMN_GUIDELINE:   
				case IMN_OPENSTATUSWINDOW:   
				case IMN_SETCANDIDATEPOS:   
				case IMN_SETCOMPOSITIONFONT:   
				case IMN_SETCOMPOSITIONWINDOW:   
				case IMN_SETSTATUSWINDOWPOS: 
				{
					break;
				}
			}

			break;
		}

		case WM_IME_SETCONTEXT:
		{
			lParam &= ~ISC_SHOWUICOMPOSITIONWINDOW;
			lParam &= ~ISC_SHOWUIGUIDELINE;
			lParam &= ~ISC_SHOWUIALLCANDIDATEWINDOW;
			break;
		}

		case WM_IME_CHAR:
		{
			return 0;
		}

		case WM_SIZE:
		{
			int width = LOWORD(lParam);
			int height = HIWORD(lParam);

			if (width > 0 && height > 0)
			{
				g_iVideoWidth = width;
				g_iVideoHeight = height;
			}

			break;
		}

		case WM_ACTIVATEAPP:
		{
			g_fActiveApp = LOWORD(wParam);
			g_fMinimized = HIWORD(wParam) != WA_INACTIVE;

			if (g_fActiveApp && !g_fMinimized)
			{
				VID_CheckGamma();
				VID_ClearMouseState();

				if (s_bCorsorLock)
					VID_LockCursor();
			}
			else
			{
				VID_RestoreGamma();
			}

			break;
		}

		case WM_INPUT:
		{
			HRAWINPUT hRawInput = (HRAWINPUT)lParam;
			RAWINPUT inp;
			UINT size = sizeof(inp);

			if (!s_bSupportRelative)
				break;

			GetRawInputData(hRawInput, RID_INPUT, &inp, &size, sizeof(RAWINPUTHEADER));

			if (inp.header.dwType == RIM_TYPEMOUSE)
			{
				RAWMOUSE *mouse = &inp.data.mouse;

				if ((mouse->usFlags & 0x01) == MOUSE_MOVE_RELATIVE)
				{
					VID_MouseMotion((int)mouse->lLastX, (int)mouse->lLastY);
				}
				else
				{
					static POINT initialMousePoint = { 0, 0 };

					if (initialMousePoint.x == 0 && initialMousePoint.y == 0)
					{
						initialMousePoint.x = mouse->lLastX;
						initialMousePoint.y = mouse->lLastY;
					}

					VID_MouseMotion((int)(mouse->lLastX-initialMousePoint.x), (int)(mouse->lLastY-initialMousePoint.y));

					initialMousePoint.x = mouse->lLastX;
					initialMousePoint.y = mouse->lLastY;
				}

				VID_MouseButton(mouse->usButtonFlags);
			}

			break;
		}

		case WM_COPYDATA:
		{
			gEngfuncs.pfnClientCmd((char *)(((COPYDATASTRUCT *)lParam)->lpData));
			gEngfuncs.pfnClientCmd("\n");
			return 1;
		}
	}

	return CallWindowProc(g_WndProc, hWnd, uMsg, wParam, lParam);
}

//BOOL CALLBACK VID_EnumWindowsProc(HWND hWnd, LPARAM)
//{
//	//if((*g_pGame)->GetMainWindow())
//	//{
//	HWND hWndMain = ;
//		if (hWnd == hWndMain)
//		{
//			g_hMainWnd = hWnd;
//
//			g_hMainDC = GetDC(hWnd);
//			g_WndProc = (WNDPROC)GetWindowLong(hWnd, GWL_WNDPROC);
//
//			RAWINPUTDEVICE Rid;
//			Rid.usUsagePage = 0x01;
//			Rid.usUsage = 0x02;
//			Rid.dwFlags = RIDEV_CAPTUREMOUSE;
//			Rid.hwndTarget = hWnd;
//
//			if (RegisterRawInputDevices(&Rid, 1, sizeof(Rid)) != FALSE)
//				s_bSupportRelative = true;
//
//			SetWindowLong(hWnd, GWL_WNDPROC, (LONG)VID_MainWndProc);
//			return FALSE;
//		}
//	//}
//
//	return TRUE;
//}

HRESULT CALLBACK VID_EnumDisplayModesProc(void *lpDDSurfaceDesc, DWORD *pBPP)
{
	DWORD dwWidth = *(DWORD *)((DWORD)lpDDSurfaceDesc + 0xC);
	DWORD dwHeight = *(DWORD *)((DWORD)lpDDSurfaceDesc + 0x8);
	DWORD dwBPP = *(DWORD *)((DWORD)lpDDSurfaceDesc + 0x54);

	if (dwHeight >= 480 && dwWidth > dwHeight && dwBPP == *pBPP)
		(*g_pVideoMode)->AddMode(dwWidth, dwHeight, dwBPP);

	return 1;
}

BYTE *VID_GetVideoBuffer(void)
{
	return (BYTE *)(*g_ppvVideoBuffer);
}

HBITMAP WINAPI VID_CreateDIBSection(HDC hdc, CONST BITMAPINFO *lpbmi, UINT usage, VOID **ppvBits, HANDLE hSection, DWORD offset)
{
	if (!g_ppvVideoBuffer)
		g_ppvVideoBuffer = ppvBits;

	return g_pfnCreateDIBSection(hdc, lpbmi, usage, ppvBits, hSection, offset);
}

bool VID_GetWindowSize(int &width, int &height)
{
	RECT rc = { 0, 0, 0, 0 };

	if (GetClientRect(g_hMainWnd, &rc))
	{
		width = rc.right - rc.left;
		height = rc.bottom - rc.top;
		return true;
	}

	return false;
}

void VID_Shutdown(void)
{
	if (g_hMainDC)
		ReleaseDC(g_hMainWnd, g_hMainDC);

	VID_UnlockCursor();

	if (s_bSupportGamma)
	{
		VID_RestoreGamma();
	}

	if (s_bSupportRelative)
	{
		RAWINPUTDEVICE Rid;
		Rid.usUsagePage = 0x01; 
		Rid.usUsage = 0x02; 
		Rid.dwFlags = RIDEV_REMOVE;
		Rid.hwndTarget = NULL;

		RegisterRawInputDevices(&Rid, 1, sizeof(Rid));
	}

	s_bSupportGamma = false;
	s_bSupportRelative = false;

	//if (g_phRegisterClassA)
	//{
	//	g_pMetaHookAPI->UnHook(g_phRegisterClassA);
	//	g_phRegisterClassA = NULL;
	//}

	//if (g_phCreateWindowExA)
	//{
	//	g_pMetaHookAPI->UnHook(g_phCreateWindowExA);
	//	g_phCreateWindowExA = NULL;
	//}

	//if (g_phDestroyWindow)
	//{
	//	g_pMetaHookAPI->UnHook(g_phDestroyWindow);
	//	g_phDestroyWindow = NULL;
	//}

	//if (g_phSetCursorPos)
	//{
	//	g_pMetaHookAPI->UnHook(g_phSetCursorPos);
	//	g_phSetCursorPos = NULL;
	//}

	//if (g_phCreateDIBSection)
	//{
	//	g_pMetaHookAPI->UnHook(g_phCreateDIBSection);
	//	g_phCreateDIBSection = NULL;
	//}
}

xcommand_t g_pfnVID_RestartGame;

void __CmdFunc_VID_RestartGame(void)
{
	char szFilePath[260];
	char szFilePath2[260];
	char szCommand[260];
	char szCurDirectory[260];

	HANDLE hObject = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, "ValveHalfLifeLauncherMutex");
	if(hObject)
	{
		BOOL release = ReleaseMutex(hObject);
	}

	LPSTR lpCmd = GetCommandLineA();

	GetCurrentDirectoryA(sizeof(szCurDirectory), szCurDirectory);

	sscanf(lpCmd, "\"%[^\"]\"%[^\n]", szFilePath, szCommand);

	sprintf(szFilePath2, "\"%s\"", szFilePath);
	SHELLEXECUTEINFOA ShExecInfo;
	ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFOA);
	ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	ShExecInfo.hwnd = NULL;
	ShExecInfo.lpVerb = NULL;
	ShExecInfo.lpFile = szFilePath2;
	ShExecInfo.lpParameters = szCommand;
	ShExecInfo.lpDirectory = szCurDirectory;
	ShExecInfo.nShow = SW_SHOW;
	ShExecInfo.hInstApp = NULL;

	BOOL result = ShellExecuteExA(&ShExecInfo);

	gEngfuncs.pfnClientCmd("quit\n");
}


void VID_InstallHook(void)
{
	g_phCreateWindowExA = g_pMetaHookAPI->InlineHook((void *)CreateWindowExA, VID_CreateWindowExA, (void *&)g_pfnCreateWindowExA);
	g_phDestroyWindow = g_pMetaHookAPI->InlineHook((void *)DestroyWindow, VID_DestroyWindow, (void *&)g_pfnDestroyWindow);
	g_phSetCursorPos = g_pMetaHookAPI->InlineHook((void *)SetCursorPos, VID_SetCursorPos, (void *&)g_pfnSetCursorPos);

	if (g_iVideoMode == VIDEOMODE_SOFTWARE)
		g_phCreateDIBSection = g_pMetaHookAPI->InlineHook((void *)CreateDIBSection, VID_CreateDIBSection, (void *&)g_pfnCreateDIBSection);
}

void VID_Init(void)
{
	if (!CommandLine()->CheckParm("-nogamma"))
	{
		if (!g_bWindowed)
			s_bSupportGamma = GetDeviceGammaRamp(g_hMainDC, s_usStateRamp);
	}

	Cvar_RegisterVariable(&vid_monitorgamma);

	g_pfnVID_RestartGame = HOOK_COMMAND("_restart", VID_RestartGame);
}

void VID_Frame(void)
{
	VID_CheckGamma();
}

void VID_SetVideoMode(int width, int height, int bpp)
{
	char szCmd[256];
	sprintf(szCmd, "_setvideomode %i %i %i\n", width, height, bpp);
	gEngfuncs.pfnClientCmd(szCmd);
}

void VID_GetCurrentRenderer(char *name, int namelen, int *windowed)
{
	*windowed = g_bWindowed;

	if (name)
	{
		switch (g_iVideoMode)
		{
			case VIDEOMODE_OPENGL:
			{
				strncpy(name, "gl", namelen - 1);
				name[namelen - 1] = 0;
				break;
			}

			case VIDEOMODE_D3D:
			{
				strncpy(name, "d3d", namelen - 1);
				name[namelen - 1] = 0;
				break;
			}
		}
	}
}

const char *VID_GetCurrentRendererName(void)
{
	switch (g_iVideoMode)
	{
		case VIDEOMODE_OPENGL:
		{
			return "OpenGL";
		}

		case VIDEOMODE_D3D:
		{
			return "Direct3D";
		}
	}

	return "Unknown";
}

void VID_SetRenderer(char *renderer, bool windowed)
{
	char szCmd[256];
	sprintf(szCmd, "_setrenderer %s %s\n", renderer, windowed ? "windowed" : "fullscreen");
	gEngfuncs.pfnClientCmd(szCmd);
}

void VID_Restart(void)
{
	VID_Shutdown();

	gEngfuncs.pfnClientCmd("_restart\n");
}

bool VID_IsWindowed(void)
{
	return g_bWindowed;
}

void VID_HideWindow(void)
{
	ShowWindow(g_hMainWnd, SW_HIDE);
}

void VID_CloseWindow(void)
{
	g_pfnDestroyWindow(g_hMainWnd);
}