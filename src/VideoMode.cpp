#define _WIN32_WINNT 0x0501
#include <metahook.h>
#include "hooks.h"
#include "VideoMode.h"

CVideoMode_Common *g_pVideoMode = NULL;

static const char *(__fastcall *m_pfnVideoMode_GetName)(void *pthis, int) = NULL;
static bool (__fastcall *m_pfnVideoMode_Init)(void *pthis, int, void *pvInstance) = NULL;
static void (__fastcall *m_pfnVideoMode_Shutdown)(void *pthis, int) = NULL;
static bool (__fastcall *m_pfnVideoMode_AddMode)(void *pthis, int, int width, int height, int bpp) = NULL;
static struct vmode_s *(__fastcall *m_pfnVideoMode_GetCurrentMode)(void *pthis, int) = NULL;
static struct vmode_s *(__fastcall *m_pfnVideoMode_GetMode)(void *pthis, int, int num) = NULL;
static int (__fastcall *m_pfnVideoMode_GetModeCount)(void *pthis, int) = NULL;
static bool (__fastcall *m_pfnVideoMode_IsWindowedMode)(void *pthis, int) = NULL;
static bool (__fastcall *m_pfnVideoMode_GetInitialized)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnVideoMode_SetInitialized)(void *pthis, int, bool init) = NULL;
static void (__fastcall *m_pfnVideoMode_UpdateWindowPosition)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnVideoMode_FlipScreen)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnVideoMode_RestoreVideo)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnVideoMode_ReleaseVideo)(void *pthis, int) = NULL;

class CVideoMode_Hook : public CVideoMode_Common
{
public:
	virtual const char *GetName(void);
	virtual bool Init(void *pvInstance);
	virtual void Shutdown(void);
	virtual bool AddMode(int width, int height, int bpp);
	virtual struct vmode_s *GetCurrentMode(void);
	virtual struct vmode_s *GetMode(int num);
	virtual int GetModeCount(void);
	virtual bool IsWindowedMode(void);
	virtual bool GetInitialized(void);
	virtual void SetInitialized(bool init);
	virtual void UpdateWindowPosition(void);
	virtual void FlipScreen(void);
	virtual void RestoreVideo(void);
	virtual void ReleaseVideo(void);
};

CVideoMode_Hook g_VideoMode_Hook;

const char *CVideoMode_Hook::GetName(void)
{
	return m_pfnVideoMode_GetName(this, 0);
}

bool CVideoMode_Hook::Init(void *pvInstance)
{
	return m_pfnVideoMode_Init(this, 0, pvInstance);
}

void CVideoMode_Hook::Shutdown(void)
{
	m_pfnVideoMode_Shutdown(this, 0);
}

bool CVideoMode_Hook::AddMode(int width, int height, int bpp)
{
	return m_pfnVideoMode_AddMode(this, 0, width, height, bpp);
}

struct vmode_s *CVideoMode_Hook::GetCurrentMode(void)
{
	return m_pfnVideoMode_GetCurrentMode(this, 0);
}

struct vmode_s *CVideoMode_Hook::GetMode(int num)
{
	return m_pfnVideoMode_GetMode(this, 0, num);
}

int CVideoMode_Hook::GetModeCount(void)
{
	return m_pfnVideoMode_GetModeCount(this, 0);
}

bool CVideoMode_Hook::IsWindowedMode(void)
{
	return m_pfnVideoMode_IsWindowedMode(this, 0);
}

bool CVideoMode_Hook::GetInitialized(void)
{
	return m_pfnVideoMode_GetInitialized(this, 0);
}

void CVideoMode_Hook::SetInitialized(bool init)
{
	m_pfnVideoMode_SetInitialized(this, 0, init);
}

void CVideoMode_Hook::UpdateWindowPosition(void)
{
	m_pfnVideoMode_UpdateWindowPosition(this, 0);
}

void CVideoMode_Hook::FlipScreen(void)
{
	m_pfnVideoMode_FlipScreen(this, 0);
}

void CVideoMode_Hook::RestoreVideo(void)
{
	m_pfnVideoMode_RestoreVideo(this, 0);
}

void CVideoMode_Hook::ReleaseVideo(void)
{
	m_pfnVideoMode_ReleaseVideo(this, 0);
}

CVideoMode_Common *VideoMode_Create(void)
{
	g_pVideoMode = g_pfnVideoMode_Create();

	if (g_pVideoMode)
	{
		DWORD *pVFTable = *(DWORD **)&g_VideoMode_Hook;

		g_pMetaHookAPI->VFTHook(g_pVideoMode, 0, 0, (void *)pVFTable[0], (void *&)m_pfnVideoMode_GetName);
		g_pMetaHookAPI->VFTHook(g_pVideoMode, 0, 1, (void *)pVFTable[1], (void *&)m_pfnVideoMode_Init);
		g_pMetaHookAPI->VFTHook(g_pVideoMode, 0, 2, (void *)pVFTable[2], (void *&)m_pfnVideoMode_Shutdown);
		g_pMetaHookAPI->VFTHook(g_pVideoMode, 0, 3, (void *)pVFTable[3], (void *&)m_pfnVideoMode_AddMode);
		g_pMetaHookAPI->VFTHook(g_pVideoMode, 0, 4, (void *)pVFTable[4], (void *&)m_pfnVideoMode_GetCurrentMode);
		g_pMetaHookAPI->VFTHook(g_pVideoMode, 0, 5, (void *)pVFTable[5], (void *&)m_pfnVideoMode_GetMode);
		g_pMetaHookAPI->VFTHook(g_pVideoMode, 0, 6, (void *)pVFTable[6], (void *&)m_pfnVideoMode_GetModeCount);
		g_pMetaHookAPI->VFTHook(g_pVideoMode, 0, 7, (void *)pVFTable[7], (void *&)m_pfnVideoMode_IsWindowedMode);
		g_pMetaHookAPI->VFTHook(g_pVideoMode, 0, 8, (void *)pVFTable[8], (void *&)m_pfnVideoMode_GetInitialized);
		g_pMetaHookAPI->VFTHook(g_pVideoMode, 0, 9, (void *)pVFTable[9], (void *&)m_pfnVideoMode_SetInitialized);
		g_pMetaHookAPI->VFTHook(g_pVideoMode, 0, 10, (void *)pVFTable[10], (void *&)m_pfnVideoMode_UpdateWindowPosition);
		g_pMetaHookAPI->VFTHook(g_pVideoMode, 0, 11, (void *)pVFTable[11], (void *&)m_pfnVideoMode_FlipScreen);
		g_pMetaHookAPI->VFTHook(g_pVideoMode, 0, 12, (void *)pVFTable[12], (void *&)m_pfnVideoMode_RestoreVideo);
		g_pMetaHookAPI->VFTHook(g_pVideoMode, 0, 13, (void *)pVFTable[13], (void *&)m_pfnVideoMode_ReleaseVideo);
		
	}

	return g_pVideoMode;
}