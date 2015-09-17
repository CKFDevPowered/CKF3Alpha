#define VIDEOITEM_RELATIVEMOUSE 1
#define VIDEOITEM_MONITORGAMMA 2

extern DWORD g_iVideoMode;
extern int g_iVideoWidth, g_iVideoHeight;
extern bool g_bWindowed;
extern BOOL g_fActiveApp;
extern BOOL g_fMinimized;

extern HWND g_hMainWnd;
extern HDC g_hMainDC;

BOOL CALLBACK VID_EnumWindowsProc(HWND hWnd, LPARAM);
HRESULT CALLBACK VID_EnumDisplayModesProc(void *lpDDSurfaceDesc, DWORD *pBPP);
void VID_InstallHook(void);
void VID_Init(void);
void VID_Shutdown(void);
void VID_Frame(void);
void VID_SetVideoMode(int width, int height, int bpp);
void VID_GetCurrentRenderer(char *name, int namelen, int *windowed);
const char *VID_GetCurrentRendererName(void);
void VID_SetRenderer(char *renderer, bool windowed);
void VID_Restart(void);
bool VID_IsSupportsItem(int item);
void VID_HideWindow(void);
void VID_CloseWindow(void);
void VID_LockCursor(void);
void VID_UnlockCursor(void);
void VID_HideCursor(void);
void VID_ShowCursor(void);
void VID_SetRelativeMouseMode(bool enabled);
BOOL VID_GetRelativeMouseState(int *x, int *y);
int VID_GetRelativeMouseButton(void);
void VID_ClearMouseState(void);
BOOL VID_IsMinimized(void);
void VID_BuildGamma(void);
void VID_CheckGamma(void);
void VID_RestoreGamma(void);
void VID_SetWindowed(bool state);
bool VID_IsWindowed(void);
BYTE *VID_GetVideoBuffer(void);
void VID_SetMainWindow(HWND hWnd);