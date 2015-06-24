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

CVideoMode_Common *VideoMode_Create(void);
void VideoMode_SwitchMode(int hardware, int windowed);
void VideoMode_SetVideoMode(int width, int height, int bpp);
void VideoMode_GetVideoModes(vmode_s **liststart, int *count);
void VideoMode_GetCurrentVideoMode(int *wide, int *tall, int *bpp);
void VideoMode_GetCurrentRenderer(char *name, int namelen, int *windowed, int *hdmodels, int *addons_folder);
int VideoMode_IsWindowed(void);
void VideoMode_RestoreVideo(void);

extern CVideoMode_Common *g_pVideoMode;