#pragma once

class CVideoMode_Common
{
public:
	virtual const char *GetName(void) = NULL;
	virtual bool Init(void *pvInstance) = NULL;
	virtual void Shutdown(void) = NULL;
	virtual bool AddMode(int width, int height, int bpp) = NULL;
	virtual struct vmode_s *GetCurrentMode(void) = NULL;
	virtual struct vmode_s *GetMode(int num) = NULL;
	virtual int GetModeCount(void) = NULL;
	virtual bool IsWindowedMode(void) = NULL;
	virtual bool GetInitialized(void) = NULL;
	virtual void SetInitialized(bool init) = NULL;
	virtual void UpdateWindowPosition(void) = NULL;
	virtual void FlipScreen(void) = NULL;
	virtual void RestoreVideo(void) = NULL;
	virtual void ReleaseVideo(void) = NULL;
};

class IGame
{
public:
	virtual			~IGame( void ) { }

	virtual	bool	Init( void *pvInstance ) = 0;
	virtual bool	Shutdown( void ) = 0;

	virtual bool	CreateGameWindow( void ) = 0;

	virtual void	SleepUntilInput( int time ) = 0;

	virtual HWND	GetMainWindow( void ) = 0;
	virtual HWND	*GetMainWindowAddress( void ) = 0;

	virtual void	SetWindowXY( int x, int y ) = 0;
	virtual void	SetWindowSize( int w, int h ) = 0;

	virtual void	GetWindowRect( int *x, int *y, int *w, int *h ) = 0;

	// Not Alt-Tabbed away
	virtual bool	IsActiveApp( void ) = 0;

	virtual bool	IsMultiplayer( void ) = 0;

	virtual void	PlayStartupVideos() = 0;

	virtual void	PlayAVIAndWait( const char *fileName ) = 0;

	virtual void	SetCursorVisible( bool bState ) = 0;
};

CVideoMode_Common *VideoMode_Create(void);
void VideoMode_SwitchMode(int hardware, int windowed);
void VideoMode_SetVideoMode(int width, int height, int bpp);
void VideoMode_GetVideoModes(vmode_s **liststart, int *count);
void VideoMode_GetCurrentVideoMode(int *wide, int *tall, int *bpp);
void VideoMode_GetCurrentRenderer(char *name, int namelen, int *windowed, int *hdmodels, int *addons_folder);
int VideoMode_IsWindowed(void);
void VideoMode_RestoreVideo(void);

void __fastcall LoadStartupGraphic(void *pthis, int);
void __fastcall DrawStartupGraphic_GL(void *pthis, int, HWND hWnd);
void __fastcall DrawStartupGraphic_GDI(void *pthis, int, HWND hWnd);

extern CVideoMode_Common **g_pVideoMode;
extern IGame **g_pGame;