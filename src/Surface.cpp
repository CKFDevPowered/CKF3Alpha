#define ENABLE_HTMLWINDOW
#define PROTECTED_THINGS_DISABLE

#define _WIN32_WINNT 0x0501

#include <metahook.h>
#include "plugins.h"
#include <VGUI/ISurface.h>
#include <VGUI/IInputInternal.h>
#include <VGUI/IScheme.h>
#include <vgui/IVGui.h>
#include <VGUI/Cursor.h>
#include "FontTextureCache.h"
#include "Color.h"
#include <vgui_controls/Controls.h>
#include "vgui_internal.h"
#if !defined (_X360)
#include <windows.h>
#endif
#include "HtmlWindow.h"
#include "Video.h"
#include "LoadTGA.h"
#include "qgl.h"
#include "Clip2D.h"
#include <VGUI/Vector.h>
#include <cvardef.h>

#if defined (_X360)
#include "xbox/xbox_win32stubs.h"
#endif

#include "tier1/utlvector.h"
#include "tier1/utlrbtree.h"
#include "tier1/UtlDict.h"

#include "tier0/memdbgon.h"

extern cvar_t *vgui_stenciltest;
extern cvar_t *vgui_emulatemouse;

#define MAX_TEXTURE_WIDTH 2048
#define MAX_TEXTURE_HEIGHT 2048
#define MAX_BATCHED_CHAR_VERTS 4096

using namespace vgui;

struct PaintState_t
{
	VPANEL m_pPanel;
	int m_iTranslateX;
	int m_iTranslateY;
	int m_iScissorLeft;
	int m_iScissorRight;
	int m_iScissorTop;
	int m_iScissorBottom;
};

struct CharRenderInfo
{
	wchar_t ch;

	bool valid;
	bool additive;
	bool shouldclip;

	int x, y;

	Vertex_t verts[2];
	int textureId;
	int abcA;
	int abcB;
	int abcC;
	int fontTall;
	HFont currentFont;
};

struct CBatchedCharInfo
{
	Vertex_t verts[2];
	int textureId;
};

class Texture
{
public:
	int _id;
	int _wide;
	int _tall;
	float _s0;
	float _t0;
	float _s1;
	float _t1;
	const char *_filename;
};

static void (__fastcall *m_pfnSurface_Shutdown)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnSurface_RunFrame)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnSurface_DrawSetTextFont)(void *pthis, int, HFont font) = NULL;
static void (__fastcall *m_pfnSurface_DrawUnicodeChar)(void *pthis, int, wchar_t wch) = NULL;
static void (__fastcall *m_pfnSurface_DrawUnicodeCharAdd)(void *pthis, int, wchar_t wch) = NULL;
static void (__fastcall *m_pfnSurface_DrawSetTextureFile)(void *pthis, int, int id, const char *filename, int hardwareFilter, bool forceReload) = NULL;
static void (__fastcall *m_pfnSurface_DrawSetTextureRGBA)(void *pthis, int, int id, const unsigned char *rgba, int wide, int tall, int hardwareFilter, bool forceReload) = NULL;
static void (__fastcall *m_pfnSurface_DrawSetTexture)(void *pthis, int, int id) = NULL;
static bool (__fastcall *m_pfnSurface_AddGlyphSetToFont)(void *pthis, int, HFont font, const char *windowsFontName, int tall, int weight, int blur, int scanlines, int flags, int lowRange, int highRange) = NULL;
static int (__fastcall *m_pfnSurface_GetFontTall)(void *pthis, int, HFont font) = NULL;
static void (__fastcall *m_pfnSurface_GetCharABCwide)(void *pthis, int, HFont font, int ch, int &a, int &b, int &c) = NULL;
static int (__fastcall *m_pfnSurface_GetCharacterWidth)(void *pthis, int, HFont font, int ch) = NULL;
static void (__fastcall *m_pfnSurface_GetTextSize)(void *pthis, int, HFont font, const wchar_t *text, int &wide, int &tall) = NULL;
static int (__fastcall *m_pfnSurface_GetFontAscent)(void *pthis, int, HFont font, wchar_t wch) = NULL;
static HFont (__fastcall *m_pfnSurface_CreateFont)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnSurface_DrawSetColor)(void *pthis, int, int r, int g, int b, int a) = NULL;
static void (__fastcall *m_pfnSurface_DrawSetColor2)(void *pthis, int, Color col) = NULL;
static void (__fastcall *m_pfnSurface_DrawSetTextColor)(void *pthis, int, int r, int g, int b, int a) = NULL;
static void (__fastcall *m_pfnSurface_DrawSetTextColor2)(void *pthis, int, Color col) = NULL;
static void (__fastcall *m_pfnSurface_DrawFilledRect)(void *pthis, int, int x0, int y0, int x1, int y1) = NULL;
static void (__fastcall *m_pfnSurface_DrawOutlinedRect)(void *pthis, int, int x0, int y0, int x1, int y1) = NULL;
static void (__fastcall *m_pfnSurface_DrawLine)(void *pthis, int, int x0, int y0, int x1, int y1) = NULL;
static void (__fastcall *m_pfnSurface_DrawPolyLine)(void *pthis, int, int *px, int *py, int numPoints) = NULL;
static void (__fastcall *m_pfnSurface_DrawTexturedRect)(void *pthis, int, int x0, int y0, int x1, int y1) = NULL;
static void (__fastcall *m_pfnSurface_GetScreenSize)(void *pthis, int, int &wide, int &tall) = NULL;
static IHTML *(__fastcall *m_pfnSurface_CreateHTMLWindow)(void *pthis, int, IHTMLEvents *events, VPANEL context) = NULL;
static void (__fastcall *m_pfnSurface_PaintHTMLWindow)(void *pthis, int, IHTML *htmlwin) = NULL;
static void (__fastcall *m_pfnSurface_DeleteHTMLWindow)(void *pthis, int, IHTML *htmlwin) = NULL;
static bool (__fastcall *m_pfnSurface_HasFocus)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnSurface_SetAllowHTMLJavaScript)(void *pthis, int, bool state) = NULL;
static bool (__fastcall *m_pfnSurface_SupportsFeature)(void *pthis, int, ISurface::SurfaceFeature_e feature) = NULL;
static bool (__fastcall *m_pfnSurface_AddCustomFontFile)(void *pthis, int, const char *fontFileName) = NULL;
static void (__fastcall *m_pfnSurface_SurfaceSetCursorPos)(void *pthis, int, int x, int y) = NULL;
static void (__fastcall *m_pfnSurface_SetCursor)(void *pthis, int, HCursor cursor) = NULL;
static void (__fastcall *m_pfnSurface_UnlockCursor)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnSurface_LockCursor)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnSurface_CalculateMouseVisible)(void *pthis, int) = NULL;
static bool (__fastcall *m_pfnSurface_IsCursorVisible)(void *pthis, int) = NULL;
static VPANEL (__fastcall *m_pfnSurface_GetEmbeddedPanel)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnSurface_SetEmbeddedPanel)(void *pthis, int, VPANEL pPanel) = NULL;
static void (__fastcall *m_pfnSurface_PaintTraverse)(void *pthis, int, VPANEL panel) = NULL;
static void (__fastcall *m_pfnSurface_PushMakeCurrent)(void *pthis, int, VPANEL panel, bool useInsets) = NULL;
static void (__fastcall *m_pfnSurface_PopMakeCurrent)(void *pthis, int, VPANEL panel) = NULL;
static void (__fastcall *m_pfnSurface_DrawFlushText)(void *pthis, int) = NULL;
static void (__fastcall *m_pfnSurface_RestrictPaintToSinglePanel)(void *pthis, int, VPANEL panel) = NULL;
static void (__fastcall *m_pfnSurface_DrawPrintText)(void *pthis, int, const wchar_t *text, int textLen) = NULL;
static int (__fastcall *m_pfnSurface_CreateNewTextureID)(void *pthis, int, bool procedural) = NULL;
static bool (__fastcall *m_pfnSurface_IsTextureIDValid)(void *pthis, int, int id) = NULL;
static void (__fastcall *m_pfnSurface_DrawGetTextureSize)(void *pthis, int, int id, int &wide, int &tall) = NULL;
static bool (__fastcall *m_pfnSurface_ShouldPaintChildPanel)(void *pthis, int, VPANEL childPanel) = NULL;
static void (__fastcall *m_pfnSurface_DeleteTextureByID)(void *pthis, int, int id) = NULL;
static void (__fastcall *m_pfnSurface_DrawUpdateRegionTextureBGRA)(void *pthis, int, int nTextureID, int x, int y, const unsigned char *pchData, int wide, int tall) = NULL;
static void (__fastcall *m_pfnSurface_CreatePopup)(void *pthis, int, VPANEL panel, bool minimised, bool showTaskbarIcon, bool disabled, bool mouseInput, bool kbInput) = NULL;
static int (__fastcall *m_pfnSurface_GetPopupCount)(void *pthis, int) = NULL;
static VPANEL (__fastcall *m_pfnSurface_GetPopup)(void *pthis, int, int index) = NULL;
static void (__fastcall *m_pfnSurface_MovePopupToFront)(void *pthis, int, VPANEL panel) = NULL;
static void (__fastcall *m_pfnSurface_MovePopupToBack)(void *pthis, int, VPANEL panel) = NULL;

class CSurface : public ISurface
{
public:
	CSurface(void);
	~CSurface(void);

public:
	virtual void Shutdown(void);
	virtual void RunFrame(void);
	virtual VPANEL GetEmbeddedPanel(void);
	virtual void SetEmbeddedPanel(VPANEL pPanel);
	virtual void PushMakeCurrent(VPANEL panel, bool useInsets);
	virtual void PopMakeCurrent(VPANEL panel);
	virtual void DrawSetColor(int r, int g, int b, int a);
	virtual void DrawSetColor(Color col);
	virtual void DrawFilledRect(int x0, int y0, int x1, int y1);
	virtual void DrawOutlinedRect(int x0, int y0, int x1, int y1);
	virtual void DrawLine(int x0, int y0, int x1, int y1);
	virtual void DrawPolyLine(int *px, int *py, int numPoints);
	virtual void DrawSetTextFont(HFont font);
	virtual void DrawSetTextColor(int r, int g, int b, int a);
	virtual void DrawSetTextColor(Color col);
	virtual void DrawSetTextPos(int x, int y);
	virtual void DrawGetTextPos(int &x, int &y);
	virtual void DrawPrintText(const wchar_t *text, int textLen);
	virtual void DrawUnicodeChar(wchar_t wch);
	virtual void DrawUnicodeCharAdd(wchar_t wch);
	virtual void DrawFlushText(void);
	virtual IHTML *CreateHTMLWindow(IHTMLEvents *events, VPANEL context);
	virtual void PaintHTMLWindow(IHTML *htmlwin);
	virtual void DeleteHTMLWindow(IHTML *htmlwin);
	virtual void DrawSetTextureFile(int id, const char *filename, int hardwareFilter, bool forceReload);
	virtual void DrawSetTextureRGBA(int id, const unsigned char *rgba, int wide, int tall, int hardwareFilter, bool forceReload);
	virtual void DrawSetTexture(int id);
	virtual void DrawGetTextureSize(int id, int &wide, int &tall);
	virtual void DrawTexturedRect(int x0, int y0, int x1, int y1);
	virtual bool IsTextureIDValid(int id);
	virtual int CreateNewTextureID(bool procedural = false);
	virtual void GetScreenSize(int &wide, int &tall);
	virtual void SetAsTopMost(VPANEL panel, bool state);
	virtual void BringToFront(VPANEL panel);
	virtual void SetForegroundWindow(VPANEL panel);
	virtual void SetPanelVisible(VPANEL panel, bool state);
	virtual void SetMinimized(VPANEL panel, bool state);
	virtual bool IsMinimized(VPANEL panel);
	virtual void FlashWindow(VPANEL panel, bool state);
	virtual void SetTitle(VPANEL panel, const wchar_t *title);
	virtual void SetAsToolBar(VPANEL panel, bool state);
	virtual void CreatePopup(VPANEL panel, bool minimised, bool showTaskbarIcon = true, bool disabled = false, bool mouseInput = true, bool kbInput = true);
	virtual void SwapBuffers(VPANEL panel);
	virtual void Invalidate(VPANEL panel);
	virtual void SetCursor(HCursor cursor);
	virtual bool IsCursorVisible(void);
	virtual void ApplyChanges(void);
	virtual bool IsWithin(int x, int y);
	virtual bool HasFocus(void);
	virtual bool SupportsFeature(SurfaceFeature_e feature);
	virtual void RestrictPaintToSinglePanel(VPANEL panel);
	virtual void SetModalPanel(VPANEL panel);
	virtual VPANEL GetModalPanel(void);
	virtual void UnlockCursor(void);
	virtual void LockCursor(void);
	virtual void SetTranslateExtendedKeys(bool state);
	virtual VPANEL GetTopmostPopup(void);
	virtual void SetTopLevelFocus(VPANEL panel);
	virtual HFont CreateFont(void);
	virtual bool AddGlyphSetToFont(HFont font, const char *windowsFontName, int tall, int weight, int blur, int scanlines, int flags, int lowRange, int highRange);
	virtual bool AddCustomFontFile(const char *fontFileName);
	virtual int GetFontTall(HFont font);
	virtual void GetCharABCwide(HFont font, int ch, int &a, int &b, int &c);
	virtual int GetCharacterWidth(HFont font, int ch);
	virtual void GetTextSize(HFont font, const wchar_t *text, int &wide, int &tall);
	virtual VPANEL GetNotifyPanel(void);
	virtual void SetNotifyIcon(VPANEL context, HTexture icon, VPANEL panelToReceiveMessages, const char *text);
	virtual void PlaySound(const char *fileName);
	virtual int GetPopupCount(void);
	virtual VPANEL GetPopup(int index);
	virtual bool ShouldPaintChildPanel(VPANEL childPanel);
	virtual bool RecreateContext(VPANEL panel);
	virtual void AddPanel(VPANEL panel);
	virtual void ReleasePanel(VPANEL panel);
	virtual void MovePopupToFront(VPANEL panel);
	virtual void MovePopupToBack(VPANEL panel);
	virtual void SolveTraverse(VPANEL panel, bool forceApplySchemeSettings = false);
	virtual void PaintTraverse(VPANEL panel);
	virtual void EnableMouseCapture(VPANEL panel, bool state);
	virtual void GetWorkspaceBounds(int &x, int &y, int &wide, int &tall);
	virtual void GetAbsoluteWindowBounds(int &x, int &y, int &wide, int &tall);
	virtual void GetProportionalBase(int &width, int &height);
	virtual void CalculateMouseVisible(void);
	virtual bool NeedKBInput(void);
	virtual bool HasCursorPosFunctions(void);
	virtual void SurfaceGetCursorPos(int &x, int &y);
	virtual void SurfaceSetCursorPos(int x, int y);
	virtual void DrawTexturedPolygon(int *p, int n);
	virtual int GetFontAscent(HFont font, wchar_t wch);
	virtual void SetAllowHTMLJavaScript(bool state);

public:
	void DrawSetAlphaMultiplier(float alpha);
	float DrawGetAlphaMultiplier(void);

	void DrawFilledRectFade(int x0, int y0, int x1, int y1, unsigned int alpha0, unsigned int alpha1, bool bHorizontal);

	bool DrawGetTextureFile(int id, char *filename, int maxlen);
	int DrawGetTextureId(char const *filename);
	void DeleteTextureByID(int id);

	void DrawTexturedPolygon(int n, float *pVertices);
	void DrawTexturedSubRect(int x0, int y0, int x1, int y1, float texs0, float text0, float texs1, float text1);

	void DrawSetTextureRGB(int id, const unsigned char *rgb, int wide, int tall, int hardwareFilter, bool forceReload);
	void DrawSetTextureBGR(int id, const unsigned char *rgba, int wide, int tall, int hardwareFilter, bool forceReload);
	void DrawSetTextureBGRA(int id, const unsigned char *rgba, int wide, int tall, int hardwareFilter, bool forceReload);
	void DrawUpdateRegionTextureRGB(int nTextureID, int x, int y, const unsigned char *pchData, int wide, int tall);
	void DrawUpdateRegionTextureRGBA(int nTextureID, int x, int y, const unsigned char *pchData, int wide, int tall);
	void DrawUpdateRegionTextureBGR(int nTextureID, int x, int y, const unsigned char *pchData, int wide, int tall);
	void DrawUpdateRegionTextureBGRA(int nTextureID, int x, int y, const unsigned char *pchData, int wide, int tall);

	bool AddBitmapFontFile(const char *fontFileName);
	void SetBitmapFontName(const char *pName, const char *pFontFilename);
	const char *GetBitmapFontName(const char *pName);

public:
	void StartDrawing(void);
	void FinishDrawing(void);
	void SetupPaintState(const PaintState_t &paintState);

	void InitVertex(Vertex_t &vertex, int x, int y, float u, float v);
	void DrawTexturedLineInternal(const Vertex_t &a, const Vertex_t &b);
	void DrawTexturedLine(const Vertex_t &a, const Vertex_t &b);
	void DrawTexturedPolyLine(const Vertex_t *p, int n);
	void DrawQuad(const Vertex_t &ul, const Vertex_t &lr, unsigned char *pColor);
	void DrawQuadBlend(const Vertex_t &ul, const Vertex_t &lr, unsigned char *pColor);
	bool DrawGetUnicodeCharRenderInfo(wchar_t ch, CharRenderInfo &info);
	void DrawRenderCharInternal(const CharRenderInfo &info);
	void DrawRenderCharFromInfo(const CharRenderInfo &info);
	void DrawQuadArray(int quadCount, Vertex_t *pVerts, unsigned char *pColor);
	void DrawTexturedPolygon(int n, Vertex_t *pVertices);

	bool IsPanelUnderRestrictedPanel(VPANEL panel);
};

CSurface g_Surface;
static bool g_bInDrawing;
static bool g_bIsLoadingDXT;

bool TextureLessFunc(const Texture &lhs, const Texture &rhs);

static float m_flAlphaMultiplier;
static bool m_bAllowJavaScript;
static CUtlRBTree<Texture, int> m_VGuiSurfaceTextures(0, 128, TextureLessFunc);
static CUtlDict<IImage *, unsigned short> m_FileTypeImages;
static CUtlVector<HtmlWindow *> m_HtmlWindows;
static byte m_TextureBuffer[MAX_TEXTURE_WIDTH * MAX_TEXTURE_HEIGHT * 4];
static HCursor m_hCurrentCursor;
static int m_nTranslateX, m_nTranslateY;
static int m_pSurfaceExtents[4];
static unsigned char m_DrawColor[4];
static unsigned char m_DrawTextColor[4];
static int m_pDrawTextPos[2];
static VPANEL m_pRestrictedPanel;
static CUtlVector<PaintState_t> m_PaintStateStack;
static int m_iBoundTexture;
static Texture *m_pBoundTexture;
static CUtlVector<CBatchedCharInfo> m_BatchedCharInfos(0, 256);
static HFont m_hCurrentFont;
static IImage *m_pMouseTexture;
static byte *m_pSwapBuffer = NULL;
static CUtlVector<CUtlSymbol> m_CustomFontFileNames;
static CUtlVector<CUtlSymbol> m_BitmapFontFileNames;
static CUtlDict<int, int> m_BitmapFontFileMapping;

CSurface::CSurface(void)
{
	CoInitialize(NULL);

	m_DrawColor[0] = m_DrawColor[1] = m_DrawColor[2] = m_DrawColor[3] = 255;
	m_nTranslateX = m_nTranslateY = 0;

	EnableScissor(false);
	SetScissorRect(0, 0, 100000, 100000);

	m_flAlphaMultiplier = 1.0;

	m_bAllowJavaScript = false;
	m_iBoundTexture = 0;

	m_pDrawTextPos[0] = m_pDrawTextPos[1] = 0;
	m_DrawTextColor[0] = m_DrawTextColor[1] = m_DrawTextColor[2] = m_DrawTextColor[3] = 255;

	m_hCurrentCursor = dc_none;
	m_hCurrentFont = NULL;

	m_pRestrictedPanel = NULL;

	m_pMouseTexture = NULL;
	m_pBoundTexture = NULL;
}

CSurface::~CSurface(void)
{
	//CoUninitialize();
}

bool TextureLessFunc(const Texture &lhs, const Texture &rhs)
{
	return lhs._id < rhs._id;
}

Texture *GetTextureById(int id)
{
	Texture findTex = { id };
	int index = m_VGuiSurfaceTextures.Find(findTex);

	if (m_VGuiSurfaceTextures.IsValidIndex(index))
		return &m_VGuiSurfaceTextures[index];

	return NULL;
}

Texture *AllocTextureForId(int id)
{
	Texture newTex = { id };
	int index = m_VGuiSurfaceTextures.Insert(newTex);
	return &m_VGuiSurfaceTextures[index];
}

int GetNumTextures(void)
{
	return m_VGuiSurfaceTextures.Count();
}

void CSurface::Shutdown(void)
{
	if (m_pSwapBuffer)
		delete m_pSwapBuffer;

#if defined (ENABLE_HTMLWINDOW)
	for (int i = 0; i < m_HtmlWindows.Size(); i++)
		delete m_HtmlWindows[i];
#endif
	for (int i = m_FileTypeImages.First(); i != m_FileTypeImages.InvalidIndex(); i = m_FileTypeImages.Next(i))
		delete m_FileTypeImages[i];

	m_HtmlWindows.RemoveAll();
	m_FileTypeImages.RemoveAll();
	m_iBoundTexture = 0;
	m_pBoundTexture = NULL;

	for (int i = 0; i < m_VGuiSurfaceTextures.MaxElement(); i++)
	{
		if (!m_VGuiSurfaceTextures.IsValidIndex(i))
			continue;
	}

#if !defined (_X360)
	for (int i = 0; i < m_CustomFontFileNames.Count(); i++)
 	{
		int nRetries = 0;

		while (::RemoveFontResource(m_CustomFontFileNames[i].String()) && (nRetries < 10))
		{
			nRetries++;
			Msg("Removed font resource %s on attempt %d.\n", m_CustomFontFileNames[i].String(), nRetries);
		}
 	}
#endif

	m_pfnSurface_Shutdown(this, 0);

 	m_CustomFontFileNames.RemoveAll();
	m_BitmapFontFileNames.RemoveAll();
	m_BitmapFontFileMapping.RemoveAll();

	FontManager().ClearAllFonts();
}

void CSurface::RunFrame(void)
{
	m_pfnSurface_RunFrame(this, 0);
}

void CSurface::StartDrawing(void)
{
	g_bInDrawing = true;

	m_iBoundTexture = 0;
	m_pBoundTexture = NULL;

	m_pSurfaceExtents[0] = 0;
	m_pSurfaceExtents[1] = 0;
	m_pSurfaceExtents[2] = g_iVideoWidth;
	m_pSurfaceExtents[3] = g_iVideoHeight;

	EnableScissor(true);

	m_nTranslateX = 0;
	m_nTranslateY = 0;
}

void CSurface::FinishDrawing(void)
{
	EnableScissor(false);

	g_bInDrawing = false;
}

VPANEL CSurface::GetEmbeddedPanel(void)
{
	return m_pfnSurface_GetEmbeddedPanel(this, 0);
}

void CSurface::SetEmbeddedPanel(VPANEL pPanel)
{
	m_pfnSurface_SetEmbeddedPanel(this, 0, pPanel);
}

void CSurface::SetupPaintState(const PaintState_t &paintState)
{
	m_nTranslateX = paintState.m_iTranslateX;
	m_nTranslateY = paintState.m_iTranslateY;

	SetScissorRect(paintState.m_iScissorLeft, paintState.m_iScissorTop, paintState.m_iScissorRight, paintState.m_iScissorBottom);
}

void CSurface::PushMakeCurrent(VPANEL panel, bool useInsets)
{
	int insets[4] = { 0, 0, 0, 0 };
	int absExtents[4];
	int clipRect[4];

	if (insets)
		g_pVGuiPanel->GetInset(panel, insets[0], insets[1], insets[2], insets[3]);

	g_pVGuiPanel->GetAbsPos(panel, absExtents[0], absExtents[1]);

	int wide, tall;
	g_pVGuiPanel->GetSize(panel, wide, tall);
	absExtents[2] = absExtents[0] + wide;
	absExtents[3] = absExtents[1] + tall;

	g_pVGuiPanel->GetClipRect(panel, clipRect[0], clipRect[1], clipRect[2], clipRect[3]);

	int i = m_PaintStateStack.AddToTail();
	PaintState_t &paintState = m_PaintStateStack[i];
	paintState.m_pPanel = panel;

	paintState.m_iTranslateX = 0;
	paintState.m_iTranslateY = 0;

	paintState.m_iScissorLeft = clipRect[0] - m_pSurfaceExtents[0] - (insets[0] + (absExtents[0] - m_pSurfaceExtents[0]));
	paintState.m_iScissorTop = clipRect[1] - m_pSurfaceExtents[1] - (insets[1] + (absExtents[1] - m_pSurfaceExtents[1]));
	paintState.m_iScissorRight = clipRect[2] - m_pSurfaceExtents[0] - (insets[0] + (absExtents[0] - m_pSurfaceExtents[0]));
	paintState.m_iScissorBottom = clipRect[3] - m_pSurfaceExtents[1] - (insets[1] + (absExtents[1] - m_pSurfaceExtents[1]));

	SetupPaintState(paintState);

	m_pfnSurface_PushMakeCurrent(this, 0, panel, useInsets);
}

void CSurface::PopMakeCurrent(VPANEL panel)
{
	if (m_BatchedCharInfos.Count())
	{
		DrawFlushText();
	}

	int top = m_PaintStateStack.Count() - 1;

	Assert(top >= 0);
	Assert(m_PaintStateStack[top].m_pPanel == panel);

	m_PaintStateStack.Remove(top);

	if (top > 0)
		SetupPaintState(m_PaintStateStack[top - 1]);

	m_pfnSurface_PopMakeCurrent(this, 0, panel);
}

void CSurface::DrawSetColor(int r, int g, int b, int a)
{
	m_DrawColor[0] = (unsigned char)r;
	m_DrawColor[1] = (unsigned char)g;
	m_DrawColor[2] = (unsigned char)b;
	m_DrawColor[3] = (unsigned char)(a * m_flAlphaMultiplier);

	m_pfnSurface_DrawSetColor(this, 0, r, g, b, a);
}

void CSurface::DrawSetColor(Color col)
{
	DrawSetColor(col[0], col[1], col[2], col[3]);
}

void CSurface::InitVertex(Vertex_t &vertex, int x, int y, float u, float v)
{
	vertex.m_Position.Init(x + m_nTranslateX, y + m_nTranslateY);
	vertex.m_TexCoord.Init(u, v);
}

void CSurface::DrawTexturedLineInternal(const Vertex_t &a, const Vertex_t &b)
{
	if (m_DrawColor[3] == 0)
		return;

	Vertex_t verts[2] = { a, b };
	Vertex_t clippedVerts[2];

	verts[0].m_Position.x += m_nTranslateX;
	verts[0].m_Position.y += m_nTranslateY;

	verts[1].m_Position.x += m_nTranslateX;
	verts[1].m_Position.y += m_nTranslateY;

	if (!ClipLine(verts, clippedVerts))
		return;

	if (g_iVideoMode == VIDEOMODE_SOFTWARE)
	{

	}
	else
	{
		if (m_iBoundTexture)
			qglEnable(GL_TEXTURE_2D);
		else
			qglDisable(GL_TEXTURE_2D);

		qglColor4ub(m_DrawColor[0], m_DrawColor[1], m_DrawColor[2], m_DrawColor[3]);
		qglBegin(GL_LINES);
		qglTexCoord2fv(clippedVerts[0].m_TexCoord.Base());
		qglVertex2f(clippedVerts[0].m_Position.x, clippedVerts[0].m_Position.y);
		qglTexCoord2fv(clippedVerts[1].m_TexCoord.Base());
		qglVertex2f(clippedVerts[1].m_Position.x, clippedVerts[1].m_Position.y);
		qglEnd();
		qglEnable(GL_TEXTURE_2D);
	}
}

void CSurface::DrawQuadArray(int quadCount, Vertex_t *pVerts, unsigned char *pColor)
{
	if (g_iVideoMode == VIDEOMODE_SOFTWARE)
	{
	}
	else
	{
		if (m_iBoundTexture)
			qglEnable(GL_TEXTURE_2D);
		else
			qglDisable(GL_TEXTURE_2D);

		qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		qglEnable(GL_BLEND);
		qglColor4ub(pColor[0], pColor[1], pColor[2], pColor[3]);

		if (g_iVideoMode == VIDEOMODE_D3D)
		{
			for (int i = 0; i < quadCount; ++i)
			{
				Vertex_t ulc, lrc;
				Vertex_t &ul = pVerts[2 * i];
				Vertex_t &lr = pVerts[2 * i + 1];

				if (!ClipRect(ul, lr, &ulc, &lrc))
					continue;

				qglBegin(GL_QUADS);
				qglTexCoord2f(ulc.m_TexCoord.x, ulc.m_TexCoord.y);
				qglVertex2f(ulc.m_Position.x, ulc.m_Position.y);
				qglTexCoord2f(lrc.m_TexCoord.x, ulc.m_TexCoord.y);
				qglVertex2f(lrc.m_Position.x, ulc.m_Position.y);
				qglTexCoord2f(lrc.m_TexCoord.x, lrc.m_TexCoord.y);
				qglVertex2f(lrc.m_Position.x, lrc.m_Position.y);
				qglTexCoord2f(ulc.m_TexCoord.x, lrc.m_TexCoord.y);
				qglVertex2f(ulc.m_Position.x, lrc.m_Position.y);
				qglEnd();
			}
		}
		else
		{
			Vertex_t *pCacheVerts = (Vertex_t *)stackalloc((quadCount * 4) * sizeof(Vertex_t));
			int iCacheCount = 0;

			for (int i = 0; i < quadCount; ++i)
			{
				Vertex_t ulc, lrc;
				Vertex_t &ul = pVerts[2 * i];
				Vertex_t &lr = pVerts[2 * i + 1];

				if (!ClipRect(ul, lr, &ulc, &lrc))
					continue;

				pCacheVerts[iCacheCount + 0].m_TexCoord = Vector2D(ulc.m_TexCoord.x, ulc.m_TexCoord.y);
				pCacheVerts[iCacheCount + 0].m_Position = Vector2D(ulc.m_Position.x, ulc.m_Position.y);
				pCacheVerts[iCacheCount + 1].m_TexCoord = Vector2D(lrc.m_TexCoord.x, ulc.m_TexCoord.y);
				pCacheVerts[iCacheCount + 1].m_Position = Vector2D(lrc.m_Position.x, ulc.m_Position.y);
				pCacheVerts[iCacheCount + 2].m_TexCoord = Vector2D(lrc.m_TexCoord.x, lrc.m_TexCoord.y);
				pCacheVerts[iCacheCount + 2].m_Position = Vector2D(lrc.m_Position.x, lrc.m_Position.y);
				pCacheVerts[iCacheCount + 3].m_TexCoord = Vector2D(ulc.m_TexCoord.x, lrc.m_TexCoord.y);
				pCacheVerts[iCacheCount + 3].m_Position = Vector2D(ulc.m_Position.x, lrc.m_Position.y);
				iCacheCount += 4;
			}

			if (iCacheCount)
			{
				qglEnableClientState(GL_VERTEX_ARRAY);
				qglEnableClientState(GL_TEXTURE_COORD_ARRAY);
				qglVertexPointer(3, GL_FLOAT, sizeof(Vertex_t), &pCacheVerts[0].m_Position.x);
				qglTexCoordPointer(2, GL_FLOAT, sizeof(Vertex_t), &pCacheVerts[0].m_TexCoord.x);
				qglDrawArrays(GL_QUADS, 0, iCacheCount);
				qglDisableClientState(GL_VERTEX_ARRAY);
				qglDisableClientState(GL_TEXTURE_COORD_ARRAY);
			}

			stackfree(pCacheVerts);
		}

		qglEnable(GL_TEXTURE_2D);
		qglDisable(GL_BLEND);
	}
}

void CSurface::DrawFilledRect(int x0, int y0, int x1, int y1)
{
	if (m_DrawColor[3] == 0)
		return;

	Vertex_t rect[2];
	Vertex_t clippedRect[2];

	InitVertex(rect[0], x0, y0, 0, 0);
	InitVertex(rect[1], x1, y1, 0, 0);

	if (!ClipRect(rect[0], rect[1], &clippedRect[0], &clippedRect[1]))
		return;

	DrawSetTexture(0);
	DrawQuad(clippedRect[0], clippedRect[1], m_DrawColor);
}

void CSurface::DrawFilledRectFade(int x0, int y0, int x1, int y1, unsigned int alpha0, unsigned int alpha1, bool bHorizontal)
{
	float alphaScale = m_DrawColor[3] / 255.f;
	unsigned char colors[4][4] = { 0 };

	alpha0 *= alphaScale;
	alpha1 *= alphaScale;

	if (alpha0 == 0 && alpha1 == 0)
		return;

	Vertex_t rect[2];
	Vertex_t clippedRect[2];

	InitVertex(rect[0], x0, y0, 0, 0);
	InitVertex(rect[1], x1, y1, 0, 0);

	if (!ClipRect(rect[0], rect[1], &clippedRect[0], &clippedRect[1]))
		return;

	DrawSetTexture(0);

	for (int i = 0; i < 4; i++)
	{
		Q_memcpy(colors[i], m_DrawColor, 3);
	}

	unsigned char nAlpha0 = (alpha0 & 0xFF);
	unsigned char nAlpha1 = (alpha1 & 0xFF);

	if (bHorizontal)
	{
		colors[0][3] = nAlpha0;
		colors[1][3] = nAlpha1;
		colors[2][3] = nAlpha1;
		colors[3][3] = nAlpha0;
	}
	else
	{
		colors[0][3] = nAlpha0;
		colors[1][3] = nAlpha0;
		colors[2][3] = nAlpha1;
		colors[3][3] = nAlpha1;
	}

	if (g_iVideoMode == VIDEOMODE_SOFTWARE)
	{
	}
	else
	{
		qglDisable(GL_TEXTURE_2D);
		qglBegin(GL_QUADS);
		qglColor4ub(colors[0][0], colors[0][1], colors[0][2], colors[0][3]);
		qglTexCoord2f(clippedRect[0].m_TexCoord.x, clippedRect[0].m_TexCoord.y);
		qglVertex2f(clippedRect[0].m_Position.x, clippedRect[0].m_Position.y);
		qglColor4ub(colors[1][0], colors[1][1], colors[1][2], colors[1][3]);
		qglTexCoord2f(clippedRect[1].m_TexCoord.x, clippedRect[1].m_TexCoord.y);
		qglVertex2f(clippedRect[1].m_Position.x, clippedRect[1].m_Position.y);
		qglColor4ub(colors[2][0], colors[2][1], colors[2][2], colors[2][3]);
		qglTexCoord2f(clippedRect[2].m_TexCoord.x, clippedRect[2].m_TexCoord.y);
		qglVertex2f(clippedRect[2].m_Position.x, clippedRect[2].m_Position.y);
		qglColor4ub(colors[3][0], colors[3][1], colors[3][2], colors[3][3]);
		qglTexCoord2f(clippedRect[3].m_TexCoord.x, clippedRect[3].m_TexCoord.y);
		qglVertex2f(clippedRect[3].m_Position.x, clippedRect[3].m_Position.y);
		qglEnd();
		qglEnable(GL_TEXTURE_2D);
	}
}

void CSurface::DrawOutlinedRect(int x0, int y0, int x1, int y1)
{
	if (m_DrawColor[3] == 0)
		return;

	DrawFilledRect(x0, y0, x1, y0 + 1);
	DrawFilledRect(x0, y1 - 1, x1, y1);
	DrawFilledRect(x0, y0 + 1, x0 + 1, y1 - 1);
	DrawFilledRect(x1 - 1, y0 + 1, x1, y1 - 1);
}

void CSurface::DrawLine(int x0, int y0, int x1, int y1)
{
	if (m_DrawColor[3] == 0)
		return;

	Vertex_t verts[2];
	verts[0].Init(Vector2D(x0, y0), Vector2D(0, 0));
	verts[1].Init(Vector2D(x1, y1), Vector2D(1, 1));

	DrawSetTexture(0);
	DrawTexturedLineInternal(verts[0], verts[1]);
}

void CSurface::DrawPolyLine(int *px, int *py, int numPoints)
{
	if (m_DrawColor[3] == 0)
		return;

	if (g_iVideoMode == VIDEOMODE_SOFTWARE)
	{
	}
	else
	{
		qglDisable(GL_TEXTURE_2D);
		qglColor4ub(m_DrawColor[0], m_DrawColor[1], m_DrawColor[2], m_DrawColor[3]);
		qglBegin(GL_LINES);

		for (int i = 0; i < numPoints ; i++)
		{
			int inext = (i + 1) % numPoints;

			Vertex_t verts[2];
			Vertex_t clippedVerts[2];

			int x0, y0, x1, y1;

			x0 = px[i];
			x1 = px[inext];
			y0 = py[i];
			y1 = py[inext];

			InitVertex(verts[0], x0, y0, 0, 0);
			InitVertex(verts[1], x1, y1, 1, 1);

			if (!ClipLine(verts, clippedVerts))
				continue;

			qglTexCoord2fv(clippedVerts[0].m_TexCoord.Base());
			qglVertex2f(clippedVerts[0].m_Position.x, clippedVerts[0].m_Position.y);
			qglTexCoord2fv(clippedVerts[1].m_TexCoord.Base());
			qglVertex2f(clippedVerts[1].m_Position.x, clippedVerts[1].m_Position.y);
		}

		qglEnd();
		qglEnable(GL_TEXTURE_2D);
	}
}

void CenterText(HDC hDC, int x, int y, LPCTSTR szFace, LPCTSTR szMessage, int point)
{
    HFONT hFont = CreateFontA(- point * GetDeviceCaps(hDC, LOGPIXELSY) / 72,
							 0, 0, 0, FW_BOLD, TRUE, FALSE, FALSE, 
							 ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, 
							 PROOF_QUALITY, VARIABLE_PITCH, szFace);
//  assert(hFont);

    HGDIOBJ hOld = SelectObject(hDC, hFont);

    SetTextAlign(hDC, TA_CENTER | TA_BASELINE);

    SetBkMode(hDC, TRANSPARENT);
    SetTextColor(hDC, RGB(0, 0, 0xFF));
    TextOut(hDC, x, y, szMessage, _tcslen(szMessage));

    SelectObject(hDC, hOld);
    DeleteObject(hFont);
}

static LRESULT CALLBACK staticProc(HWND hwnd,UINT msg,WPARAM wparam,LPARAM lparam)
{
	return NULL;
}

void DrawNullBackground( void *hHDC, int w, int h )
{
	if ( IsX360() )
		return;

	HDC hdc = (HDC)hHDC;

	// Show a message if running without renderer..
	//if ( CommandLine()->FindParm( "-noshaderapi" ) )
	//{
		HFONT fnt = CreateFontA( -18, 
		 0,
		 0,
		 0,
		 FW_NORMAL,
		 FALSE,
		 FALSE,
		 FALSE,
		 ANSI_CHARSET,
		 OUT_TT_PRECIS,
		 CLIP_DEFAULT_PRECIS,
		 ANTIALIASED_QUALITY,
		 DEFAULT_PITCH,
		 "Arial" );

		HFONT oldFont = (HFONT)SelectObject( hdc, fnt );
		int oldBkMode = SetBkMode( hdc, TRANSPARENT );
		COLORREF oldFgColor = SetTextColor( hdc, RGB( 255, 255, 255 ) );

		HBRUSH br = CreateSolidBrush( RGB( 0, 0, 0  ) );
		HBRUSH oldBr = (HBRUSH)SelectObject( hdc, br );
		Rectangle( hdc, 0, 0, w, h );
		
		RECT rc;
		rc.left = 0;
		rc.top = 0;
		rc.right = w;
		rc.bottom = h;

		DrawText( hdc, "Running with -noshaderapi", -1, &rc, DT_NOPREFIX | DT_VCENTER | DT_CENTER | DT_SINGLELINE  );

		rc.top = rc.bottom - 30;

		/*if ( host_state.worldmodel != NULL )
		{
			rc.left += 10;
			DrawText( hdc, modelloader->GetName( host_state.worldmodel ), -1, &rc, DT_NOPREFIX | DT_VCENTER | DT_SINGLELINE  );
		}*/

		SetTextColor( hdc, oldFgColor );

		SelectObject( hdc, oldBr );
		SetBkMode( hdc, oldBkMode );
		SelectObject( hdc, oldFont );

		DeleteObject( br );
		DeleteObject( fnt );
	//}
}

void BlitGraphicToHDC(HDC hdc, byte *rgba, int imageWidth, int imageHeight, int x0, int y0, int x1, int y1)
{
	if ( IsX360() )
		return;

	int x = x0;
	int y = y0;
	int wide = x1 - x0;
	int tall = y1 - y0;

	// Needs to be a multiple of 4
	int dibwide = ( wide + 3 ) & ~3;

	Assert(rgba);
	int texwby4 = imageWidth << 2;

	void *destBits = NULL;

	HBITMAP bm;
	BITMAPINFO bmi;
	Q_memset( &bmi, 0, sizeof( bmi ) );

	BITMAPINFOHEADER *hdr = &bmi.bmiHeader;

	hdr->biSize = sizeof( *hdr );
	hdr->biWidth = dibwide;
	hdr->biHeight = -tall;  // top down bitmap
	hdr->biBitCount = 24;
	hdr->biPlanes = 1;
	hdr->biCompression = BI_RGB;
	hdr->biSizeImage = dibwide * tall * 3;
	hdr->biXPelsPerMeter = 3780;
	hdr->biYPelsPerMeter = 3780;

	// Create a "source" DC
	HDC tempDC = CreateCompatibleDC( hdc );

	// Create the dibsection bitmap
	bm = CreateDIBSection
	(
		tempDC,						// handle to DC
		&bmi,						// bitmap data
		DIB_RGB_COLORS,             // data type indicator
		&destBits,					// bit values
		NULL,						// handle to file mapping object
		0							// offset to bitmap bit values
	);

	DWORD dwErr = GetLastError();
	assert(dwErr == 0);

	// Select it into the source DC
	HBITMAP oldBitmap = (HBITMAP)SelectObject( tempDC, bm );

	// Setup for bilinaer filtering. If we don't do this filter here, there will be a big
	// annoying pop when it switches to the vguimatsurface version of the background.
	// We leave room for 14 bits of integer precision, so the image can be up to 16k x 16k.
	const int BILINEAR_FIX_SHIFT = 17;
	const int BILINEAR_FIX_MUL = (1 << BILINEAR_FIX_SHIFT);

	#define FIXED_BLEND( a, b, out, frac ) \
		out[0] = (a[0]*frac + b[0]*(BILINEAR_FIX_MUL-frac)) >> BILINEAR_FIX_SHIFT; \
		out[1] = (a[1]*frac + b[1]*(BILINEAR_FIX_MUL-frac)) >> BILINEAR_FIX_SHIFT; \
		out[2] = (a[2]*frac + b[2]*(BILINEAR_FIX_MUL-frac)) >> BILINEAR_FIX_SHIFT;

	float eps = 0.001f;
	float uMax = imageWidth - 1 - eps;
	float vMax = imageHeight - 1 - eps;

	int fixedBilinearV = 0;
	int bilinearUInc = (int)( (uMax / (dibwide-1)) * BILINEAR_FIX_MUL );
	int bilinearVInc = (int)( (vMax / (tall-1)) * BILINEAR_FIX_MUL );

	for ( int v = 0; v < tall; v++ )
	{
		int iBilinearV = fixedBilinearV >> BILINEAR_FIX_SHIFT;
		int fixedFractionV = fixedBilinearV & (BILINEAR_FIX_MUL-1);
		fixedBilinearV += bilinearVInc;

		int fixedBilinearU = 0;
		byte *dest = (byte *)destBits + ( ( y + v ) * dibwide + x ) * 3;

		for ( int u = 0; u < dibwide; u++, dest+=3 )
		{
			int iBilinearU = fixedBilinearU >> BILINEAR_FIX_SHIFT;
			int fixedFractionU = fixedBilinearU & (BILINEAR_FIX_MUL-1);
			fixedBilinearU += bilinearUInc;
		
			Assert( iBilinearU >= 0 && iBilinearU+1 < imageWidth );
			Assert( iBilinearV >= 0 && iBilinearV+1 < imageHeight );

			byte *srcTopLine    = rgba + iBilinearV * texwby4;
			byte *srcBottomLine = rgba + (iBilinearV+1) * texwby4;

			byte *xsrc[4] = {
				srcTopLine + (iBilinearU+0)*4,	  srcTopLine + (iBilinearU+1)*4,
				srcBottomLine + (iBilinearU+0)*4, srcBottomLine + (iBilinearU+1)*4 	};

			int topColor[3], bottomColor[3], finalColor[3];
			FIXED_BLEND( xsrc[1], xsrc[0], topColor, fixedFractionU );
			FIXED_BLEND( xsrc[3], xsrc[2], bottomColor, fixedFractionU );
			FIXED_BLEND( bottomColor, topColor, finalColor, fixedFractionV );

			// Windows wants the colors in reverse order.
			dest[0] = finalColor[2];
			dest[1] = finalColor[1];
			dest[2] = finalColor[0];
		}
	}
	
	// Now do the Blt
	BitBlt( hdc, 0, 0, dibwide, tall, tempDC, 0, 0, SRCCOPY );

	// This only draws if running -noshaderapi
	DrawNullBackground( hdc, dibwide, tall );

	// Restore the old Bitmap
	SelectObject( tempDC, oldBitmap );

	// Destroy the temporary DC
	DeleteDC( tempDC );

	// Destroy the DIBSection bitmap
	DeleteObject( bm );

	//double elapsed = Plat_FloatTime() - st;

	//COM_TimestampedLog( "BlitGraphicToHDC: new ver took %.4f", elapsed );
}

void CSurface::DrawQuad(const Vertex_t &ul, const Vertex_t &lr, unsigned char *pColor)
{
	if (g_iVideoMode == VIDEOMODE_SOFTWARE)
	{
		static bool reg = false;
		static bool draw = false;

		if (!reg)
		{
			WNDCLASS staticWndclass = { NULL };

			memset( &staticWndclass,0,sizeof(staticWndclass) );
			staticWndclass.style = CS_PARENTDC | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
			staticWndclass.lpfnWndProc = staticProc;
			staticWndclass.hInstance = GetModuleHandle(NULL);
			staticWndclass.lpszClassName = "Surface";

			::RegisterClass( &staticWndclass );
			reg = true;
		}

		static HDC hDC, hDC2;
		static HBITMAP hRet;
		static VOID *dib = NULL;

		if (!draw)
		{
			DWORD style = WS_POPUP | WS_CLIPCHILDREN;
			DWORD style_ex = WS_EX_TOOLWINDOW;
			HWND hWnd = CreateWindowEx(style_ex, "Surface", "", style, 100, 100, g_iVideoWidth, g_iVideoHeight, g_hMainWnd, NULL, GetModuleHandle(NULL), NULL);
			HDC hParentDC = CreateCompatibleDC(NULL);

			hDC = hParentDC;

			/*BITMAPINFOHEADER bitmapInfoHeader;
			memset(&bitmapInfoHeader, 0, sizeof(bitmapInfoHeader));
			bitmapInfoHeader.biSize = sizeof(bitmapInfoHeader);
			bitmapInfoHeader.biWidth = g_iVideoWidth;
			bitmapInfoHeader.biHeight = -g_iVideoHeight;
			bitmapInfoHeader.biPlanes = 1;
			bitmapInfoHeader.biBitCount = 24;
			bitmapInfoHeader.biCompression = BI_RGB;
			bitmapInfoHeader.biSizeImage = g_iVideoWidth * g_iVideoHeight * 3;

			hRet = CreateDIBSection(hDC, (BITMAPINFO*)&bitmapInfoHeader, DIB_RGB_COLORS, &dib, NULL, 0);

			DWORD dwErr = GetLastError();
			assert(dwErr == 0);*/

			hDC2 = GetDC(hWnd);

			draw = true;
		}

		HDC hMainDC = GetDC(g_hMainWnd);
		//HDC hDC = CreateCompatibleDC(hParentDC);//GetDC(g_hMainWnd);
/*
		PAINTSTRUCT ps;
		//::BeginPaint(g_hMainWnd,&ps);

		//unsigned char *rgba = (unsigned char *)( (unsigned char *)dib + sizeof( BITMAPINFOHEADER ) + 256 * sizeof( RGBQUAD ) );

		// Copy raw data
		for (int j = 0; j < g_iVideoHeight; j++)
		{ 
			for (int i = 0; i <g_iVideoWidth; i++)
			{
				int y = (g_iVideoHeight - j - 1);

				int offs = ( y * g_iVideoWidth + i);
				int offsdest = (j * g_iVideoWidth + i) * 3;
				//unsigned char *src = ((unsigned char *)rgba) + offs;
				unsigned char *dst = ((unsigned char*)dib) + offsdest;

				dst[0] = 255;
				dst[1] = 255;
				dst[2] = 0;
				//dst[3] = (unsigned char)255;
			}
		}

		//HBITMAP hBitmap = CreateCompatibleBitmap(hDC, g_iVideoWidth, g_iVideoHeight);
		HBITMAP hBitmap = hRet;
		HBITMAP oldbmp = (HBITMAP)SelectObject(hDC, hBitmap);

		SetBkColor(hDC, RGB(255, 0, 255));

		RECT rect = { 0, 0, g_iVideoWidth, g_iVideoHeight };
		ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rect, NULL, 0, NULL);

		wchar_t t[] = { L"IGAN" };
		SetTextColor(hDC, RGB(255, 0, 255));
		ExtTextOutW(hDC, 100, 100, 0, NULL, t, wcslen(t), NULL);

		RECT rc;
		SetRect(&rc, 0, 0, 800, 600);
		::DrawText(hDC, "Center Line Text", -1, &rc, DT_VCENTER | DT_SINGLELINE | DT_CENTER);
		
		for (int x = 0; x < 100; x++)
		{
			for (int y = 0; y < 100; y++)
				SetPixel(hMainDC, x, y, RGB(255, 0, 255));
		}

		DWORD dwErr = GetLastError();
		assert(dwErr == 0);

		BitBlt(hMainDC, 0, 0, g_iVideoWidth, g_iVideoHeight, hDC, 0, 0, SRCCOPY);
		//::SwapBuffers(hMainDC);

		if (OpenClipboard(GetDesktopWindow()))
		{
			EmptyClipboard();

			if (hBitmap)
				SetClipboardData(CF_BITMAP, hBitmap);

			CloseClipboard();
		}
*/
		static unsigned char rgba[100 * 100 * 4];
		memset(rgba, 0, sizeof(rgba));
		BlitGraphicToHDC(hMainDC, rgba, 100, 100, 100, 100, 200, 200);

		//SelectObject(hDC, oldbmp);
		ReleaseDC(g_hMainWnd, hMainDC);
		//::EndPaint(g_hMainWnd,&ps);
		/*CenterText(hDC, 0,
			0,
			"System", "BBB", 72);*/

		//ReleaseDC(hWnd, hParentDC);
		//DeleteDC(hDC);
	}
	else
	{
		if (m_iBoundTexture)
			qglEnable(GL_TEXTURE_2D);
		else
			qglDisable(GL_TEXTURE_2D);

		qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		qglEnable(GL_BLEND);
		qglColor4ub(pColor[0], pColor[1], pColor[2], pColor[3]);
		qglBegin(GL_QUADS);
		qglTexCoord2f(ul.m_TexCoord.x, ul.m_TexCoord.y);
		qglVertex2f(ul.m_Position.x, ul.m_Position.y);
		qglTexCoord2f(lr.m_TexCoord.x, ul.m_TexCoord.y);
		qglVertex2f(lr.m_Position.x, ul.m_Position.y);
		qglTexCoord2f(lr.m_TexCoord.x, lr.m_TexCoord.y);
		qglVertex2f(lr.m_Position.x, lr.m_Position.y);
		qglTexCoord2f(ul.m_TexCoord.x, lr.m_TexCoord.y);
		qglVertex2f(ul.m_Position.x, lr.m_Position.y);
		qglEnd();
		qglEnable(GL_TEXTURE_2D);
		qglDisable(GL_BLEND);
	}
}

void CSurface::DrawQuadBlend(const Vertex_t &ul, const Vertex_t &lr, unsigned char *pColor)
{
	if (g_iVideoMode == VIDEOMODE_SOFTWARE)
	{
	}
	else
	{
		if (m_iBoundTexture)
			qglEnable(GL_TEXTURE_2D);
		else
			qglDisable(GL_TEXTURE_2D);

		qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
		qglBlendFunc(GL_SRC_ALPHA, GL_ONE);
		qglEnable(GL_BLEND);
		qglEnable(GL_ALPHA_TEST);
		qglColor4ub(pColor[0], pColor[1], pColor[2], pColor[3]);
		qglBegin(GL_QUADS);
		qglTexCoord2f(ul.m_TexCoord.x, ul.m_TexCoord.y);
		qglVertex2f(ul.m_Position.x, ul.m_Position.y);
		qglTexCoord2f(lr.m_TexCoord.x, ul.m_TexCoord.y);
		qglVertex2f(lr.m_Position.x, ul.m_Position.y);
		qglTexCoord2f(lr.m_TexCoord.x, lr.m_TexCoord.y);
		qglVertex2f(lr.m_Position.x, lr.m_Position.y);
		qglTexCoord2f(ul.m_TexCoord.x, lr.m_TexCoord.y);
		qglVertex2f(ul.m_Position.x, lr.m_Position.y);
		qglEnd();
		qglEnable(GL_TEXTURE_2D);
		qglDisable(GL_BLEND);
		qglDisable(GL_ALPHA_TEST);
	}
}

void CSurface::DrawTexturedPolygon(int n, Vertex_t *pVertices)
{
	if ((n == 0) || (m_DrawColor[3] == 0))
		return;

	Vertex_t **ppClippedVerts;
	int iCount = ClipPolygon(n, pVertices, m_nTranslateX, m_nTranslateY, &ppClippedVerts);

	if (iCount <= 0)
		return;

	if (g_iVideoMode == VIDEOMODE_SOFTWARE)
	{
	}
	else
	{
		qglEnable(GL_TEXTURE_2D);
		qglColor4ub(m_DrawColor[0], m_DrawColor[1], m_DrawColor[2], m_DrawColor[3]);
		qglBegin(GL_POLYGON);

		for (int i = 0; i < iCount; ++i)
		{
			qglTexCoord2f(ppClippedVerts[i]->m_TexCoord.x, ppClippedVerts[i]->m_TexCoord.y);
			qglVertex2f(ppClippedVerts[i]->m_Position.x, ppClippedVerts[i]->m_Position.y);
		}

		qglEnd();
	}
}

void CSurface::DrawSetTextFont(HFont font)
{
	m_hCurrentFont = font;
	m_pfnSurface_DrawSetTextFont(this, 0, font);
}

void CSurface::DrawSetTextColor(int r, int g, int b, int a)
{
	int adjustedAlpha = (a * m_flAlphaMultiplier);

	if (r != m_DrawTextColor[0] || g != m_DrawTextColor[1] || b != m_DrawTextColor[2] || adjustedAlpha != m_DrawTextColor[3])
	{
		DrawFlushText();

		m_DrawTextColor[0] = (unsigned char)r;
		m_DrawTextColor[1] = (unsigned char)g;
		m_DrawTextColor[2] = (unsigned char)b;
		m_DrawTextColor[3] = (unsigned char)adjustedAlpha;
	}

	m_pfnSurface_DrawSetTextColor(this, 0, r, g, b, a);
}

void CSurface::DrawSetTextColor(Color col)
{
	DrawSetTextColor(col[0], col[1], col[2], col[3]);
}

void CSurface::DrawSetTextPos(int x, int y)
{
	m_pDrawTextPos[0] = x;
	m_pDrawTextPos[1] = y;

	g_pSurface->DrawSetTextPos(x, y);
}

void CSurface::DrawGetTextPos(int &x, int &y)
{
	g_pSurface->DrawGetTextPos(x, y);
}

void CSurface::DrawPrintText(const wchar_t *text, int iTextLen)
{
	if (!text)
		return;

	if (!m_hCurrentFont)
		return;

	int x, y;
	DrawGetTextPos(x, y);

	x += m_nTranslateX;
	y += m_nTranslateY;

	int iTall = GetFontTall(m_hCurrentFont);
	int iLastTexId = -1;

	int iCount = 0;
	Vertex_t *pQuads = (Vertex_t *)stackalloc((2 * iTextLen) * sizeof(Vertex_t));

	int iTotalWidth = 0;

	for (int i = 0; i < iTextLen; ++i)
	{
		wchar_t ch = text[i];

		bool bUnderlined = FontManager().GetFontUnderlined(m_hCurrentFont);
		int abcA, abcB, abcC;
		GetCharABCwide(m_hCurrentFont, ch, abcA, abcB, abcC);
		iTotalWidth += abcA;

		int iWide = abcB;

		if (bUnderlined)
		{
			iWide += (abcA + abcC);
			x-= abcA;
		}

		if (!iswspace(ch) || bUnderlined)
		{
			int iTexId = 0;
			float *texCoords = NULL;

			if (!g_FontTextureCache.GetTextureForChar(m_hCurrentFont, ch, &iTexId, &texCoords))
				continue;

			Assert(texCoords);

			if (iTexId != iLastTexId)
			{
				if (iCount)
				{
					DrawSetTexture(iLastTexId);
					DrawQuadArray(iCount, pQuads, m_DrawTextColor);
					iCount = 0;
				}

				iLastTexId = iTexId;
			}

 			Vertex_t &ul = pQuads[2 * iCount];
 			Vertex_t &lr = pQuads[2 * iCount + 1];
			iCount++;

			ul.m_Position.x = x + iTotalWidth;
			ul.m_Position.y = y;
			lr.m_Position.x = ul.m_Position.x + iWide;
			lr.m_Position.y = ul.m_Position.y + iTall;

			ul.m_TexCoord[0] = texCoords[0];
			ul.m_TexCoord[1] = texCoords[1];
			lr.m_TexCoord[0] = texCoords[2];
			lr.m_TexCoord[1] = texCoords[3];
		}

		iTotalWidth += iWide + abcC;

		if (bUnderlined)
		{
			iTotalWidth -= abcC;
		}
	}

	if (iCount)
	{
		DrawSetTexture(iLastTexId);
		DrawQuadArray(iCount, pQuads, m_DrawTextColor);
	}

	DrawSetTextPos(x + iTotalWidth, y);

	stackfree(pQuads);
}

void CSurface::DrawUnicodeChar(wchar_t wch)
{
	CharRenderInfo info;
	info.additive = false;

	if (DrawGetUnicodeCharRenderInfo(wch, info))
		DrawRenderCharFromInfo(info);
}

void CSurface::DrawUnicodeCharAdd(wchar_t wch)
{
	CharRenderInfo info;
	info.additive = true;

	if (DrawGetUnicodeCharRenderInfo(wch, info))
		DrawRenderCharFromInfo(info);
}

bool CSurface::DrawGetUnicodeCharRenderInfo(wchar_t ch, CharRenderInfo &info)
{
	info.valid = false;

	if (!m_hCurrentFont)
	{
		return info.valid;
	}

	info.valid = true;
	info.ch = ch;

	info.currentFont = m_hCurrentFont;
	info.fontTall = GetFontTall(m_hCurrentFont);

	DrawGetTextPos(info.x, info.y);
	GetCharABCwide(m_hCurrentFont, ch, info.abcA, info.abcB, info.abcC);

	bool bUnderlined = FontManager().GetFontUnderlined(m_hCurrentFont);

	if (!bUnderlined)
	{
		info.x += info.abcA;
	}

	info.textureId = 0;
	float *texCoords = NULL;

	if (!g_FontTextureCache.GetTextureForChar(m_hCurrentFont, ch, &info.textureId, &texCoords))
	{
		info.valid = false;
		return info.valid;
	}

	int fontWide = info.abcB;

	if (bUnderlined)
	{
		fontWide += (info.abcA + info.abcC);
		info.x-= info.abcA;
	}

	InitVertex(info.verts[0], info.x, info.y, texCoords[0], texCoords[1]);
	InitVertex(info.verts[1], info.x + fontWide, info.y + info.fontTall, texCoords[2], texCoords[3]);

	info.shouldclip = true;
	return info.valid;
}

void CSurface::DrawRenderCharInternal(const CharRenderInfo &info)
{
	if (m_DrawTextColor[3] == 0)
		return;

	Vertex_t clippedRect[2];

	if (info.shouldclip)
	{
		if (!ClipRect(info.verts[0], info.verts[1], &clippedRect[0], &clippedRect[1]))
			return;
	}
	else
	{
		clippedRect[0] = info.verts[0];
		clippedRect[1] = info.verts[1];
	}

	if (m_BatchedCharInfos.Count() > MAX_BATCHED_CHAR_VERTS)
		DrawFlushText();

	if (info.additive)
	{
		DrawSetTexture(info.textureId);
		DrawQuadBlend(clippedRect[0], clippedRect[1], m_DrawTextColor);
	}
	else
	{
		CBatchedCharInfo batchedCharInfo;
		batchedCharInfo.verts[0] = clippedRect[0];
		batchedCharInfo.verts[1] = clippedRect[1];
		batchedCharInfo.textureId = info.textureId;
		m_BatchedCharInfos.AddToTail(batchedCharInfo);
	}
}

void CSurface::DrawRenderCharFromInfo(const CharRenderInfo &info)
{
	if (!info.valid)
		return;

	DrawRenderCharInternal(info);
	DrawSetTextPos(info.x + (info.abcB + info.abcC), info.y);
}

void CSurface::DrawFlushText(void)
{
	if (!m_BatchedCharInfos.Count())
		return;

	for (int i = 0; i < m_BatchedCharInfos.Count(); i++)
	{
		DrawSetTexture(m_BatchedCharInfos[i].textureId);
		DrawQuad(m_BatchedCharInfos[i].verts[0], m_BatchedCharInfos[i].verts[1], m_DrawTextColor);
	}

	m_BatchedCharInfos.RemoveAll();
}

IHTML *CSurface::CreateHTMLWindow(IHTMLEvents *events, VPANEL context)
{
#if defined (ENABLE_HTMLWINDOW)
	VPANEL parent = g_pIPanel->GetParent(context);

	if (!parent)
		return NULL;

	HtmlWindow *IE = new HtmlWindow(events, context, g_hMainWnd, m_bAllowJavaScript, SupportsFeature(DIRECT_HWND_RENDER));
	IE->SetVisible(g_pIPanel->IsVisible(parent));

	m_HtmlWindows.AddToTail(IE);
	return dynamic_cast<IHTML *>(IE);
#else
	Assert(0);
	return NULL;
#endif
}

void CSurface::PaintHTMLWindow(IHTML *htmlwin)
{
#if defined (ENABLE_HTMLWINDOW)
	HtmlWindow *IE = static_cast<HtmlWindow *>(htmlwin);

	if (IE)
	{
		HDC hdc = ::GetDC(g_hMainWnd);
		IE->OnPaint(hdc);
		::ReleaseDC(g_hMainWnd, hdc);
	}
#endif
}

void CSurface::DeleteHTMLWindow(IHTML *htmlwin)
{
#if defined (ENABLE_HTMLWINDOW)
	HtmlWindow *IE = dynamic_cast<HtmlWindow *>(htmlwin);

	if (IE)
	{
		m_HtmlWindows.FindAndRemove(IE);
		delete IE;
	}
#elif !defined (_X360)
#error "GameUI now NEEDS the HTML component!!"
#endif
}

bool CSurface::DrawGetTextureFile(int id, char *filename, int maxlen)
{
	Texture *texture = GetTextureById(id);

	if (!texture)
		return false;

	Q_strncpy(filename, texture->_filename, maxlen);
	return true;
}

int CSurface::DrawGetTextureId(char const *filename)
{
	int i = m_VGuiSurfaceTextures.FirstInorder();

	while (i != m_VGuiSurfaceTextures.InvalidIndex())
	{
		Texture *texture = &m_VGuiSurfaceTextures[i];

		if (texture->_filename)
		{
			if (!Q_stricmp(filename, texture->_filename))
				return texture->_id;
		}

		i = m_VGuiSurfaceTextures.NextInorder(i);
	}

	return -1;
}

void CSurface::DrawSetTextureFile(int id, const char *filename, int hardwareFilter, bool forceReload)
{
	Texture *texture = GetTextureById(id);

	if (!texture || !texture->_filename || (texture->_filename && stricmp(filename, texture->_filename)) || forceReload)
	{
		char name[MAX_PATH];
		Q_snprintf(name, sizeof(name), "%s.tga", filename);

		g_bIsLoadingDXT = false;

		int width, height;
		bool success = LoadImageGeneric(name, m_TextureBuffer, sizeof(m_TextureBuffer), &width, &height);

		if (!success)
		{
			const char *psz = Q_stristr(name, "vgui/");

			if (psz)
				success = LoadImageGeneric(name + strlen("vgui/"), m_TextureBuffer, sizeof(m_TextureBuffer), &width, &height);
		}

		if (!success)
		{
			Q_snprintf(name, sizeof(name), "%s.dds", filename);

			if (!LoadImageGeneric(name, m_TextureBuffer, sizeof(m_TextureBuffer), &width, &height))
			{
				const char *psz = Q_stristr(name, "vgui/");

				if (psz)
					success = LoadImageGeneric(name + strlen("vgui/"), m_TextureBuffer, sizeof(m_TextureBuffer), &width, &height);
			}
			else
			{
				success = true;
			}
		}

		if (!success)
		{
			Q_snprintf(name, sizeof(name), "%s.bmp", filename);

			if (!LoadImageGeneric(name, m_TextureBuffer, sizeof(m_TextureBuffer), &width, &height))
			{
				const char *psz = Q_stristr(name, "vgui/");

				if (psz)
					success = LoadImageGeneric(name + strlen("vgui/"), m_TextureBuffer, sizeof(m_TextureBuffer), &width, &height);
			}
			else
			{
				success = true;
			}
		}

		if (!success)
		{
			Q_snprintf(name, sizeof(name), "%s.png", filename);

			if (!LoadImageGeneric(name, m_TextureBuffer, sizeof(m_TextureBuffer), &width, &height))
			{
				const char *psz = Q_stristr(name, "vgui/");

				if (psz)
					success = LoadImageGeneric(name + strlen("vgui/"), m_TextureBuffer, sizeof(m_TextureBuffer), &width, &height);
			}
			else
			{
				success = true;
			}
		}

		if (success)
		{
			DrawSetTextureRGBA(id, m_TextureBuffer, width, height, hardwareFilter, forceReload);
		}
		else
		{
			Msg("Error: texture file '%s' does not exist or is invalid\n", filename);
			return;
		}

		if (!texture)
		{
			texture = AllocTextureForId(id);
			memset(texture, 0, sizeof(Texture));
		}

		texture->_id = id;
		texture->_filename = filename;
		texture->_wide = width;
		texture->_tall = height;
		texture->_s0 = 0;
		texture->_t0 = 0;
		texture->_s1 = 1;
		texture->_t1 = 1;
	}
}

void CSurface::DrawSetTextureRGB(int id, const unsigned char *rgba, int wide, int tall, int hardwareFilter, bool forceReload)
{
	if (g_iVideoMode != VIDEOMODE_OPENGL)
	{
		if (!m_pSwapBuffer)
			m_pSwapBuffer = new unsigned char [MAX_TEXTURE_WIDTH * MAX_TEXTURE_HEIGHT * 4];

		unsigned char *p = (unsigned char *)rgba;
		unsigned char *buf = m_pSwapBuffer;
		unsigned char *pixels = buf;

		for (int i = 0; i < wide; i++)
		{
			for (int j = 0; j < tall; j++)
			{
				pixels[0] = p[0];
				pixels[1] = p[1];
				pixels[2] = p[2];
				pixels[3] = 255;

				pixels += 4;
				p += 3;
			}
		}

		DrawSetTextureRGBA(id, buf, wide, tall, hardwareFilter, forceReload);
		return;
	}

	Texture *texture = GetTextureById(id);

	if (!texture || texture->_filename || forceReload)
	{
		if (!texture)
			texture = AllocTextureForId(id);

		texture->_id = id;
		texture->_filename = NULL;
		texture->_wide = wide;
		texture->_tall = tall;
		texture->_s0 = 0;
		texture->_t0 = 0;
		texture->_s1 = 1;
		texture->_t1 = 1;

		qglBindTexture(GL_TEXTURE_2D, id);

		if (hardwareFilter)
		{
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}

		qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		if (g_iBPP == 32)
			qglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8_EXT, wide, tall, 0, GL_RGB, GL_UNSIGNED_BYTE, rgba);
		else
			qglTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1_EXT, wide, tall, 0, GL_RGB, GL_UNSIGNED_BYTE, rgba);
	}
}

void CSurface::DrawSetTextureRGBA(int id, const unsigned char *rgba, int wide, int tall, int hardwareFilter, bool forceReload)
{
	Texture *texture = GetTextureById(id);

	if (!texture || texture->_filename || forceReload)
	{
		if (!texture)
			texture = AllocTextureForId(id);

		texture->_wide = wide;
		texture->_tall = tall;
		texture->_s0 = 0;
		texture->_t0 = 0;
		texture->_s1 = 1;
		texture->_t1 = 1;

		if (g_iVideoMode == VIDEOMODE_SOFTWARE)
		{
			m_pfnSurface_DrawSetTextureRGBA(this, 0, id, rgba, wide, tall, hardwareFilter, forceReload);
		}
		else
		{
			qglBindTexture(GL_TEXTURE_2D, id);

			if (hardwareFilter)
			{
				qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
			else
			{
				qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			}

			qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 1);
			qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

			if (g_iBPP == 32)
				qglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8_EXT, wide, tall, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
			else
				qglTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1_EXT, wide, tall, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba);
		}
	}
}

void CSurface::DrawSetTextureBGR(int id, const unsigned char *rgba, int wide, int tall, int hardwareFilter, bool forceReload)
{
	if (g_iVideoMode != VIDEOMODE_OPENGL)
	{
		if (!m_pSwapBuffer)
			m_pSwapBuffer = new unsigned char [MAX_TEXTURE_WIDTH * MAX_TEXTURE_HEIGHT * 4];

		unsigned char *p = (unsigned char *)rgba;
		unsigned char *buf = m_pSwapBuffer;
		unsigned char *pixels = buf;

		for (int i = 0; i < wide; i++)
		{
			for (int j = 0; j < tall; j++)
			{
				pixels[0] = p[2];
				pixels[1] = p[1];
				pixels[2] = p[0];
				pixels[3] = 255;

				pixels += 4;
				p += 3;
			}
		}

		DrawSetTextureRGBA(id, rgba, wide, tall, hardwareFilter, forceReload);
		return;
	}

	Texture *texture = GetTextureById(id);

	if (!texture || texture->_filename || forceReload)
	{
		if (!texture)
			texture = AllocTextureForId(id);

		texture->_wide = wide;
		texture->_tall = tall;
		texture->_s0 = 0;
		texture->_t0 = 0;
		texture->_s1 = 1;
		texture->_t1 = 1;

		qglBindTexture(GL_TEXTURE_2D, id);

		if (hardwareFilter)
		{
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}

		qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		if (g_iBPP == 32)
			qglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8_EXT, wide, tall, 0, GL_BGR, GL_UNSIGNED_BYTE, rgba);
		else
			qglTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1_EXT, wide, tall, 0, GL_BGR, GL_UNSIGNED_BYTE, rgba);
	}
}

void CSurface::DrawSetTextureBGRA(int id, const unsigned char *rgba, int wide, int tall, int hardwareFilter, bool forceReload)
{
	if (g_iVideoMode != VIDEOMODE_OPENGL)
	{
		unsigned char *p = (unsigned char *)rgba;

		for (int i = 0; i < wide; i++)
		{
			for (int j = 0; j < tall; j++)
			{
				register unsigned char tmp = p[0];

				p[0] = p[2];
				p[2] = tmp;

				p += 4;
			}
		}

		DrawSetTextureRGBA(id, rgba, wide, tall, hardwareFilter, forceReload);
		return;
	}

	Texture *texture = GetTextureById(id);

	if (!texture || texture->_filename || forceReload)
	{
		if (!texture)
			texture = AllocTextureForId(id);

		texture->_wide = wide;
		texture->_tall = tall;
		texture->_s0 = 0;
		texture->_t0 = 0;
		texture->_s1 = 1;
		texture->_t1 = 1;

		qglBindTexture(GL_TEXTURE_2D, id);

		if (hardwareFilter)
		{
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else
		{
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		}

		qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		if (g_iBPP == 32)
			qglTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8_EXT, wide, tall, 0, GL_BGRA, GL_UNSIGNED_BYTE, rgba);
		else
			qglTexImage2D(GL_TEXTURE_2D, 0, GL_RGB5_A1_EXT, wide, tall, 0, GL_BGRA, GL_UNSIGNED_BYTE, rgba);
	}
}

void CSurface::DrawUpdateRegionTextureRGB(int nTextureID, int x, int y, const unsigned char *pchData, int wide, int tall)
{
	if (g_iVideoMode != VIDEOMODE_OPENGL)
	{
		if (!m_pSwapBuffer)
			m_pSwapBuffer = new unsigned char [MAX_TEXTURE_WIDTH * MAX_TEXTURE_HEIGHT * 4];

		unsigned char *p = (unsigned char *)pchData;
		unsigned char *buf = m_pSwapBuffer;
		unsigned char *pixels = buf;

		for (int i = 0; i < wide; i++)
		{
			for (int j = 0; j < tall; j++)
			{
				pixels[0] = p[2];
				pixels[1] = p[1];
				pixels[2] = p[0];
				pixels[3] = 255;

				pixels += 4;
				p += 3;
			}
		}

		DrawUpdateRegionTextureBGRA(nTextureID, x, y, pchData, wide, tall);
		return;
	}

	Texture *texture = GetTextureById(nTextureID);

	if (texture)
	{
		qglBindTexture(GL_TEXTURE_2D, nTextureID);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		qglPixelStorei(GL_UNPACK_ROW_LENGTH, texture->_wide);
		qglTexSubImage2D(GL_TEXTURE_2D, 0, 0, y, texture->_wide, tall, GL_RGB, GL_UNSIGNED_BYTE, pchData + y * texture->_wide * 3);
		qglPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	}
}

void CSurface::DrawUpdateRegionTextureRGBA(int nTextureID, int x, int y, const unsigned char *pchData, int wide, int tall)
{
	if (g_iVideoMode != VIDEOMODE_OPENGL)
	{
		unsigned char *p = (unsigned char *)pchData;

		for (int i = 0; i < wide; i++)
		{
			for (int j = 0; j < tall; j++)
			{
				register unsigned char tmp = p[0];

				p[0] = p[2];
				p[2] = tmp;

				p += 4;
			}
		}

		DrawUpdateRegionTextureBGRA(nTextureID, x, y, pchData, wide, tall);
		return;
	}

	Texture *texture = GetTextureById(nTextureID);

	if (texture)
	{
		qglBindTexture(GL_TEXTURE_2D, nTextureID);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		qglPixelStorei(GL_UNPACK_ROW_LENGTH, texture->_wide);
		qglTexSubImage2D(GL_TEXTURE_2D, 0, 0, y, texture->_wide, tall, GL_RGBA, GL_UNSIGNED_BYTE, pchData + y * texture->_wide * 4);
		qglPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	}
}

void CSurface::DrawUpdateRegionTextureBGR(int nTextureID, int x, int y, const unsigned char *pchData, int wide, int tall)
{
	if (g_iVideoMode != VIDEOMODE_OPENGL)
	{
		if (!m_pSwapBuffer)
			m_pSwapBuffer = new unsigned char [MAX_TEXTURE_WIDTH * MAX_TEXTURE_HEIGHT * 4];

		unsigned char *p = (unsigned char *)pchData;
		unsigned char *buf = m_pSwapBuffer;
		unsigned char *pixels = buf;

		for (int i = 0; i < wide; i++)
		{
			for (int j = 0; j < tall; j++)
			{
				pixels[0] = p[0];
				pixels[1] = p[1];
				pixels[2] = p[2];
				pixels[3] = 255;

				pixels += 4;
				p += 3;
			}
		}

		DrawUpdateRegionTextureBGRA(nTextureID, x, y, pchData, wide, tall);
		return;
	}

	Texture *texture = GetTextureById(nTextureID);

	if (texture)
	{
		qglBindTexture(GL_TEXTURE_2D, nTextureID);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		qglPixelStorei(GL_UNPACK_ROW_LENGTH, texture->_wide);
		qglTexSubImage2D(GL_TEXTURE_2D, 0, 0, y, texture->_wide, tall, GL_BGR, GL_UNSIGNED_BYTE, pchData + y * texture->_wide * 4);
		qglPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	}
}

void CSurface::DrawUpdateRegionTextureBGRA(int nTextureID, int x, int y, const unsigned char *pchData, int wide, int tall)
{
	if (g_iVideoMode == VIDEOMODE_SOFTWARE)
	{
		if (m_pfnSurface_DrawUpdateRegionTextureBGRA)
			m_pfnSurface_DrawUpdateRegionTextureBGRA(this, 0, nTextureID, x, y, pchData, wide, tall);

		return;
	}

	if (g_iVideoMode == VIDEOMODE_D3D)
	{
		unsigned char *p = (unsigned char *)pchData;

		for (int i = 0; i < wide; i++)
		{
			for (int j = 0; j < tall; j++)
			{
				register unsigned char tmp = p[0];

				p[0] = p[2];
				p[2] = tmp;

				p += 4;
			}
		}

		DrawUpdateRegionTextureRGBA(nTextureID, x, y, pchData, wide, tall);
		return;
	}

	Texture *texture = GetTextureById(nTextureID);

	if (texture)
	{
		qglBindTexture(GL_TEXTURE_2D, nTextureID);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		qglTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		qglPixelStorei(GL_UNPACK_ROW_LENGTH, texture->_wide);
		qglTexSubImage2D(GL_TEXTURE_2D, 0, 0, y, texture->_wide, tall, GL_BGRA, GL_UNSIGNED_BYTE, pchData + y * texture->_wide * 4);
		qglPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	}
}

void CSurface::DrawSetTexture(int id)
{
	if (id != m_iBoundTexture)
	{
		m_iBoundTexture = id;
		m_pBoundTexture = GetTextureById(id);
	}

	m_pfnSurface_DrawSetTexture(this, 0, id);
}

void CSurface::DrawGetTextureSize(int id, int &wide, int &tall)
{
	Texture *texture = GetTextureById(id);

	if (texture)
	{
		wide = texture->_wide;
		tall = texture->_tall;
	}
}

void CSurface::DrawTexturedRect(int x0, int y0, int x1, int y1)
{
	if (m_DrawColor[3] == 0)
		return;

	if (!m_pBoundTexture)
		return;

	Vertex_t rect[2];
	Vertex_t clippedRect[2];

	InitVertex(rect[0], x0, y0, m_pBoundTexture->_s0, m_pBoundTexture->_t0);
	InitVertex(rect[1], x1, y1, m_pBoundTexture->_s1, m_pBoundTexture->_t1);

	if (!ClipRect(rect[0], rect[1], &clippedRect[0], &clippedRect[1]))
		return;

	if (g_iVideoMode == VIDEOMODE_D3D)
	{
		clippedRect[0].m_TexCoord.x += 0.5 / m_pBoundTexture->_wide;
		clippedRect[0].m_TexCoord.y += 0.5 / m_pBoundTexture->_tall;
		clippedRect[1].m_TexCoord.x -= 0.5 / m_pBoundTexture->_wide;
		clippedRect[1].m_TexCoord.y -= 0.5 / m_pBoundTexture->_tall;
	}

	if (g_iVideoMode == VIDEOMODE_SOFTWARE)
	{
	}
	else
	{
		DrawQuad(clippedRect[0], clippedRect[1], m_DrawColor);
	}
}

void CSurface::DrawTexturedSubRect(int x0, int y0, int x1, int y1, float texs0, float text0, float texs1, float text1)
{
	if (m_DrawColor[3] == 0)
		return;

	if (!m_pBoundTexture)
		return;

	float ssize = m_pBoundTexture->_s1 - m_pBoundTexture->_s0;
	float tsize = m_pBoundTexture->_t1 - m_pBoundTexture->_t0;

	texs0 = m_pBoundTexture->_s0 + texs0 * (ssize);
	texs1 = m_pBoundTexture->_s0 + texs1 * (ssize);
	text0 = m_pBoundTexture->_t0 + text0 * (tsize);
	text1 = m_pBoundTexture->_t0 + text1 * (tsize);

	Vertex_t rect[2];
	Vertex_t clippedRect[2];

	InitVertex(rect[0], x0, y0, texs0, text0);
	InitVertex(rect[1], x1, y1, texs1, text1);

	if (!ClipRect(rect[0], rect[1], &clippedRect[0], &clippedRect[1]))
		return;

	DrawQuad(clippedRect[0], clippedRect[1], m_DrawColor);
}

bool CSurface::IsTextureIDValid(int id)
{
	return (GetTextureById(id) != NULL);
}

int CSurface::CreateNewTextureID(bool procedural)
{
	return m_pfnSurface_CreateNewTextureID(this, 0, procedural);
}

void CSurface::DeleteTextureByID(int id)
{
	if (g_iVideoMode == VIDEOMODE_SOFTWARE)
	{
		if (m_pfnSurface_DeleteTextureByID)
			m_pfnSurface_DeleteTextureByID(this, 0, id);
	}
	else
	{
		qglDeleteTextures(1, (const GLuint *)&id);
	}
}

void CSurface::GetScreenSize(int &wide, int &tall)
{
	wide = g_iVideoWidth;
	tall = g_iVideoHeight;
}

void CSurface::SetAsTopMost(VPANEL panel, bool state)
{
	g_pSurface->SetAsTopMost(panel, state);
}

void CSurface::BringToFront(VPANEL panel)
{
	g_pSurface->BringToFront(panel);
}

void CSurface::SetForegroundWindow(VPANEL panel)
{
	g_pSurface->SetForegroundWindow(panel);
}

void CSurface::SetPanelVisible(VPANEL panel, bool state)
{
	g_pSurface->SetPanelVisible(panel, state);
}

void CSurface::SetMinimized(VPANEL panel, bool state)
{
	g_pSurface->SetMinimized(panel, state);
}

bool CSurface::IsMinimized(VPANEL panel)
{
	return g_pSurface->IsMinimized(panel);
}

void CSurface::FlashWindow(VPANEL panel, bool state)
{
	g_pSurface->FlashWindow(panel, state);
}

void CSurface::SetTitle(VPANEL panel, const wchar_t *title)
{
	g_pSurface->SetTitle(panel, title);
}

void CSurface::SetAsToolBar(VPANEL panel, bool state)
{
	g_pSurface->SetAsToolBar(panel, state);
}

void CSurface::CreatePopup(VPANEL panel, bool minimised, bool showTaskbarIcon, bool disabled, bool mouseInput, bool kbInput)
{
	m_pfnSurface_CreatePopup(this, 0, panel, minimised, showTaskbarIcon, disabled, mouseInput, kbInput);
}

void CSurface::SwapBuffers(VPANEL panel)
{
	g_pSurface->SwapBuffers(panel);
}

void CSurface::Invalidate(VPANEL panel)
{
	g_pSurface->Invalidate(panel);
}

void CSurface::SetCursor(HCursor cursor)
{
	if (m_hCurrentCursor != cursor)
	{
		m_hCurrentCursor = cursor;
		m_pfnSurface_SetCursor(this, 0, cursor);
	}
}

bool CSurface::IsCursorVisible(void)
{
	return m_hCurrentCursor != dc_none;
}

void CSurface::ApplyChanges(void)
{
	g_pSurface->ApplyChanges();
}

bool CSurface::IsWithin(int x, int y)
{
	return g_pSurface->IsWithin(x, y);
}

bool CSurface::HasFocus(void)
{
	bool result = m_pfnSurface_HasFocus(this, 0);

	if (!result)
	{
		HWND focus = ::GetFocus();

		for (int i = 0; i < m_HtmlWindows.Count(); i++)
		{
			if (m_HtmlWindows[i]->GetIEHWND() == focus)
				return true;
		}
	}

	return result;
}

bool CSurface::SupportsFeature(ISurface::SurfaceFeature_e feature)
{
	switch (feature)
	{
		case OUTLINE_FONTS:
		{
			if (IsX360())
				return false;

			return true;
		}
	}

	return m_pfnSurface_SupportsFeature(this, 0, feature);
}

void CSurface::RestrictPaintToSinglePanel(VPANEL panel)
{
	if (panel && m_pRestrictedPanel && m_pRestrictedPanel == input()->GetAppModalSurface())
		return;

	m_pRestrictedPanel = panel;
	m_pfnSurface_RestrictPaintToSinglePanel(this, 0, panel);
}

bool CSurface::IsPanelUnderRestrictedPanel(VPANEL panel)
{
	if (!m_pRestrictedPanel)
		return true;

	while (panel)
	{
		if (panel == m_pRestrictedPanel)
			return true;

		panel = g_pIPanel->GetParent(panel);
	}

	return false;
}

void CSurface::SetModalPanel(VPANEL panel)
{
	g_pSurface->SetModalPanel(panel);
}

VPANEL CSurface::GetModalPanel(void)
{
	return g_pSurface->GetModalPanel();
}

void CSurface::UnlockCursor(void)
{
	m_pfnSurface_UnlockCursor(this, 0);
}

void CSurface::LockCursor(void)
{
	m_pfnSurface_LockCursor(this, 0);
}

void CSurface::SetTranslateExtendedKeys(bool state)
{
	g_pSurface->SetTranslateExtendedKeys(state);
}

VPANEL CSurface::GetTopmostPopup(void)
{
	return g_pSurface->GetTopmostPopup();
}

void CSurface::SetTopLevelFocus(VPANEL panel)
{
	g_pSurface->SetTopLevelFocus(panel);
}

HFont CSurface::CreateFont(void)
{
	return FontManager().CreateFont();
}

bool CSurface::AddGlyphSetToFont(HFont font, const char *windowsFontName, int tall, int weight, int blur, int scanlines, int flags, int lowRange, int highRange)
{
	return FontManager().AddGlyphSetToFont(font, windowsFontName, tall, weight, blur, scanlines, flags, lowRange, highRange);
}

bool CSurface::AddCustomFontFile(const char *fontFileName)
{
	char fullPath[MAX_PATH];

	if (!g_pFullFileSystem->GetLocalPath(fontFileName, fullPath, sizeof(fullPath)))
	{
		Msg("Couldn't find custom font file '%s'\n", fontFileName);
		return false;
	}

	Q_strlower(fullPath);
	CUtlSymbol sym(fullPath);
	int i;

	for (i = 0; i < m_CustomFontFileNames.Count(); i++)
	{
		if (m_CustomFontFileNames[i] == sym)
			break;
	}

	if (!m_CustomFontFileNames.IsValidIndex(i))
	{
	 	m_CustomFontFileNames.AddToTail(fullPath);

		if (IsPC())
		{
			g_pFullFileSystem->GetLocalCopy(fullPath);
		}
	}

#if !defined (_X360)
	bool success = (::AddFontResource(fullPath) > 0);

	if (success)
	{
		FontManager().AddCustomFontFile(fullPath);
	}
	else
	{
		Msg("Failed to load custom font file '%s'\n", fullPath);
	}

	Assert(success);
	return success;
#else
	return true;
#endif
}

bool CSurface::AddBitmapFontFile(const char *fontFileName)
{
	if (!g_pFullFileSystem->FileExists(fontFileName))
	{
		Msg("Couldn't find bitmap font file '%s'\n", fontFileName);
		return false;
	}

	char path[MAX_PATH];
	Q_strncpy(path, fontFileName, MAX_PATH);

	Q_strlower(path);
	CUtlSymbol sym(path);
	int i;

	for (i = 0; i < m_BitmapFontFileNames.Count(); i++)
	{
		if (m_BitmapFontFileNames[i] == sym)
			break;
	}

	if (!m_BitmapFontFileNames.IsValidIndex(i))
	{
	 	m_BitmapFontFileNames.AddToTail(path);

		if (IsPC())
		{
			g_pFullFileSystem->GetLocalCopy(path);
		}
	}

	return true;
}

void CSurface::SetBitmapFontName(const char *pName, const char *pFontFilename)
{
	char fontPath[MAX_PATH];
	Q_strncpy(fontPath, pFontFilename, MAX_PATH);
	Q_strlower(fontPath);

	CUtlSymbol sym(fontPath);

	for (int i = 0; i < m_BitmapFontFileNames.Count(); i++)
	{
		if (m_BitmapFontFileNames[i] == sym)
		{
			int index = m_BitmapFontFileMapping.Find(pName);

			if (!m_BitmapFontFileMapping.IsValidIndex(index))
			{
				index = m_BitmapFontFileMapping.Insert(pName);
			}

			m_BitmapFontFileMapping.Element(index) = i;
			break;
		}
	}
}

const char *CSurface::GetBitmapFontName(const char *pName)
{
	int index = m_BitmapFontFileMapping.Find(pName);

	if (index == m_BitmapFontFileMapping.InvalidIndex())
		return "";

	return m_BitmapFontFileNames[m_BitmapFontFileMapping.Element(index)].String();
}

int CSurface::GetFontTall(HFont font)
{
	return FontManager().GetFontTall(font);
}

void CSurface::GetCharABCwide(HFont font, int ch, int &a, int &b, int &c)
{
	return FontManager().GetCharABCwide(font, ch, a, b, c);
}

int CSurface::GetCharacterWidth(HFont font, int ch)
{
	return FontManager().GetCharacterWidth(font, ch);
}

void CSurface::GetTextSize(HFont font, const wchar_t *text, int &wide, int &tall)
{
	return FontManager().GetTextSize(font, text, wide, tall);
}

VPANEL CSurface::GetNotifyPanel(void)
{
	return g_pSurface->GetNotifyPanel();
}

void CSurface::SetNotifyIcon(VPANEL context, HTexture icon, VPANEL panelToReceiveMessages, const char *text)
{
	g_pSurface->SetNotifyIcon(context, icon, panelToReceiveMessages, text);
}

void CSurface::PlaySound(const char *fileName)
{
	g_pSurface->PlaySound(fileName);
}

int CSurface::GetPopupCount(void)
{
	return m_pfnSurface_GetPopupCount(this, 0);
}

VPANEL CSurface::GetPopup(int index)
{
	return m_pfnSurface_GetPopup(this, 0, index);
}

bool CSurface::ShouldPaintChildPanel(VPANEL childPanel)
{
	return m_pfnSurface_ShouldPaintChildPanel(this, 0, childPanel);
}

bool CSurface::RecreateContext(VPANEL panel)
{
	return g_pSurface->RecreateContext(panel);
}

void CSurface::AddPanel(VPANEL panel)
{
	g_pSurface->AddPanel(panel);
}

void CSurface::ReleasePanel(VPANEL panel)
{
	g_pSurface->ReleasePanel(panel);
}

void CSurface::MovePopupToFront(VPANEL panel)
{
	return m_pfnSurface_MovePopupToFront(this, 0, panel);
}

void CSurface::MovePopupToBack(VPANEL panel)
{
	return m_pfnSurface_MovePopupToBack(this, 0, panel);
}

void CSurface::SolveTraverse(VPANEL panel, bool forceApplySchemeSettings)
{
	g_pSurface->SolveTraverse(panel, forceApplySchemeSettings);
}

void CSurface::PaintTraverse(VPANEL panel)
{
	if (!g_pIPanel->IsVisible(panel))
		return;

	bool bTopLevelDraw = false;

	if (g_bInDrawing == false)
	{
		bTopLevelDraw = true;
		StartDrawing();
	}

	int popups = GetPopupCount();

	if (panel == GetEmbeddedPanel())
	{
		for (int i = 0; i < popups; i++)
			g_pIPanel->Render_SetPopupVisible(GetPopup(i), false);

		g_pIPanel->PaintTraverse(panel, true);

		for (int i = 0; i < popups; i++)
		{
			if (g_pIPanel->Render_GetPopupVisible(GetPopup(i)))
				g_pIPanel->PaintTraverse(GetPopup(i), true);
		}

		if (IsCursorVisible() && vgui_emulatemouse && vgui_emulatemouse->value)
		{
			int x, y;
			g_pInput->GetCursorPos(x, y);

			if (!m_pMouseTexture)
				m_pMouseTexture = g_pScheme->GetImage("gfx/vgui/mouse", false);

			m_pMouseTexture->SetPos(x, y);
			m_pMouseTexture->Paint();
		}
	}
	else
	{
		if (!g_pIPanel->IsPopup(panel))
			g_pIPanel->PaintTraverse(panel, true);
	}

	if (bTopLevelDraw)
		FinishDrawing();
}

void CSurface::EnableMouseCapture(VPANEL panel, bool state)
{
	g_pSurface->EnableMouseCapture(panel, state);
}

void CSurface::GetWorkspaceBounds(int &x, int &y, int &wide, int &tall)
{
	g_pSurface->GetWorkspaceBounds(x, y, wide, tall);
}

void CSurface::GetAbsoluteWindowBounds(int &x, int &y, int &wide, int &tall)
{
	g_pSurface->GetAbsoluteWindowBounds(x, y, wide, tall);
}

void CSurface::GetProportionalBase(int &width, int &height)
{
	g_pSurface->GetProportionalBase(width, height);
}

void CSurface::CalculateMouseVisible(void)
{
	return m_pfnSurface_CalculateMouseVisible(this, 0);
}

bool CSurface::NeedKBInput(void)
{
	return g_pSurface->NeedKBInput();
}

bool CSurface::HasCursorPosFunctions(void)
{
	return g_pSurface->HasCursorPosFunctions();
}

void CSurface::SurfaceGetCursorPos(int &x, int &y)
{
	g_pSurface->SurfaceGetCursorPos(x, y);
}

void CSurface::SurfaceSetCursorPos(int x, int y)
{
	return m_pfnSurface_SurfaceSetCursorPos(this, 0, x, y);
}

void CSurface::DrawTexturedPolygon(int *p, int n)
{
	g_pSurface->DrawTexturedPolygon(p, n);
}

int CSurface::GetFontAscent(HFont font, wchar_t wch)
{
	return FontManager().GetFontAscent(font, wch);
}

void CSurface::SetAllowHTMLJavaScript(bool state)
{
	m_bAllowJavaScript = state;
}

void CSurface::DrawSetAlphaMultiplier(float alpha)
{
	m_flAlphaMultiplier = alpha;
}

float CSurface::DrawGetAlphaMultiplier(void)
{
	return m_flAlphaMultiplier;
}

void Surface_InstallHook(void)
{
	CreateInterfaceFn fnCreateInterface = g_pMetaHookAPI->GetEngineFactory();
	g_pSurface = (ISurface *)fnCreateInterface(VGUI_SURFACE_INTERFACE_VERSION, NULL);

	CSurface Surface;
	DWORD *pVFTable = *(DWORD **)&Surface;

	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 1, (void *)pVFTable[1], (void *&)m_pfnSurface_Shutdown);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 2, (void *)pVFTable[2], (void *&)m_pfnSurface_RunFrame);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 5, (void *)pVFTable[5], (void *&)m_pfnSurface_PushMakeCurrent);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 6, (void *)pVFTable[6], (void *&)m_pfnSurface_PopMakeCurrent);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 8, (void *)pVFTable[8], (void *&)m_pfnSurface_DrawSetColor);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 7, (void *)pVFTable[7], (void *&)m_pfnSurface_DrawSetColor2);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 9, (void *)pVFTable[9], (void *&)m_pfnSurface_DrawFilledRect);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 10, (void *)pVFTable[10], (void *&)m_pfnSurface_DrawOutlinedRect);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 11, (void *)pVFTable[11], (void *&)m_pfnSurface_DrawLine);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 12, (void *)pVFTable[12], (void *&)m_pfnSurface_DrawPolyLine);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 13, (void *)pVFTable[13], (void *&)m_pfnSurface_DrawSetTextFont);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 15, (void *)pVFTable[15], (void *&)m_pfnSurface_DrawSetTextColor);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 14, (void *)pVFTable[14], (void *&)m_pfnSurface_DrawSetTextColor2);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 18, (void *)pVFTable[18], (void *&)m_pfnSurface_DrawPrintText);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 19, (void *)pVFTable[19], (void *&)m_pfnSurface_DrawUnicodeChar);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 20, (void *)pVFTable[20], (void *&)m_pfnSurface_DrawUnicodeCharAdd);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 21, (void *)pVFTable[21], (void *&)m_pfnSurface_DrawFlushText);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 22, (void *)pVFTable[22], (void *&)m_pfnSurface_CreateHTMLWindow);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 23, (void *)pVFTable[23], (void *&)m_pfnSurface_PaintHTMLWindow);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 24, (void *)pVFTable[24], (void *&)m_pfnSurface_DeleteHTMLWindow);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 25, (void *)pVFTable[25], (void *&)m_pfnSurface_DrawSetTextureFile);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 26, (void *)pVFTable[26], (void *&)m_pfnSurface_DrawSetTextureRGBA);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 27, (void *)pVFTable[27], (void *&)m_pfnSurface_DrawSetTexture);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 28, (void *)pVFTable[28], (void *&)m_pfnSurface_DrawGetTextureSize);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 29, (void *)pVFTable[29], (void *&)m_pfnSurface_DrawTexturedRect);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 30, (void *)pVFTable[30], (void *&)m_pfnSurface_IsTextureIDValid);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 31, (void *)pVFTable[31], (void *&)m_pfnSurface_CreateNewTextureID);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 32, (void *)pVFTable[32], (void *&)m_pfnSurface_GetScreenSize);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 42, (void *)pVFTable[42], (void *&)m_pfnSurface_CreatePopup);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 49, (void *)pVFTable[49], (void *&)m_pfnSurface_HasFocus);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 50, (void *)pVFTable[50], (void *&)m_pfnSurface_SupportsFeature);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 51, (void *)pVFTable[51], (void *&)m_pfnSurface_RestrictPaintToSinglePanel);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 59, (void *)pVFTable[59], (void *&)m_pfnSurface_CreateFont);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 60, (void *)pVFTable[60], (void *&)m_pfnSurface_AddGlyphSetToFont);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 61, (void *)pVFTable[61], (void *&)m_pfnSurface_AddCustomFontFile);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 62, (void *)pVFTable[62], (void *&)m_pfnSurface_GetFontTall);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 63, (void *)pVFTable[63], (void *&)m_pfnSurface_GetCharABCwide);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 64, (void *)pVFTable[64], (void *&)m_pfnSurface_GetCharacterWidth);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 65, (void *)pVFTable[65], (void *&)m_pfnSurface_GetTextSize);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 69, (void *)pVFTable[69], (void *&)m_pfnSurface_GetPopupCount);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 70, (void *)pVFTable[70], (void *&)m_pfnSurface_GetPopup);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 71, (void *)pVFTable[71], (void *&)m_pfnSurface_ShouldPaintChildPanel);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 75, (void *)pVFTable[75], (void *&)m_pfnSurface_MovePopupToFront);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 76, (void *)pVFTable[76], (void *&)m_pfnSurface_MovePopupToBack);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 78, (void *)pVFTable[78], (void *&)m_pfnSurface_PaintTraverse);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 89, (void *)pVFTable[89], (void *&)m_pfnSurface_GetFontAscent);
	g_pMetaHookAPI->VFTHook(g_pSurface, 0, 90, (void *)pVFTable[90], (void *&)m_pfnSurface_SetAllowHTMLJavaScript);

	if (g_dwEngineBuildnum > 5953)
	{
		pVFTable = *(DWORD **)g_pSurface;

		m_pfnSurface_DeleteTextureByID = (void (__fastcall *)(void *, int, int))pVFTable[93];
		m_pfnSurface_DrawUpdateRegionTextureBGRA = (void (__fastcall *)(void *, int, int, int, int, const unsigned char *, int, int))pVFTable[94];
	}
}

void ISurface::DrawSetAlphaMultiplier(float alpha)
{
	g_Surface.DrawSetAlphaMultiplier(alpha);
}

float ISurface::DrawGetAlphaMultiplier(void)
{
	return g_Surface.DrawGetAlphaMultiplier();
}

void ISurface::DrawFilledRectFade(int x0, int y0, int x1, int y1, unsigned int alpha0, unsigned int alpha1, bool bHorizontal)
{
	return g_Surface.DrawFilledRectFade(x0, y0, x1, y1, alpha0, alpha1, bHorizontal);
}

bool ISurface::DrawGetTextureFile(int id, char *filename, int maxlen)
{
	return g_Surface.DrawGetTextureFile(id, filename, maxlen);
}

int ISurface::DrawGetTextureId(char const *filename)
{
	return g_Surface.DrawGetTextureId(filename);
}

void ISurface::DrawTexturedPolygon(int n, float *pVertices)
{
	g_Surface.DrawTexturedPolygon(n, (Vertex_t *)pVertices);
}

void ISurface::DrawTexturedSubRect(int x0, int y0, int x1, int y1, float texs0, float text0, float texs1, float text1)
{
	g_Surface.DrawTexturedSubRect(x0, y0, x1, y1, texs0, text0, texs1, text1);
}

void ISurface::DrawSetTextureRGB(int id, const unsigned char *rgb, int wide, int tall, int hardwareFilter, bool forceReload)
{
	g_Surface.DrawSetTextureRGB(id, rgb, wide, tall, hardwareFilter, forceReload);
}

void ISurface::DrawSetTextureBGR(int id, const unsigned char *rgba, int wide, int tall, int hardwareFilter, bool forceReload)
{
	g_Surface.DrawSetTextureBGR(id, rgba, wide, tall, hardwareFilter, forceReload);
}

void ISurface::DrawSetTextureBGRA(int id, const unsigned char *rgba, int wide, int tall, int hardwareFilter, bool forceReload)
{
	g_Surface.DrawSetTextureBGRA(id, rgba, wide, tall, hardwareFilter, forceReload);
}

void ISurface::DrawUpdateRegionTextureRGB(int nTextureID, int x, int y, const unsigned char *pchData, int wide, int tall)
{
	g_Surface.DrawUpdateRegionTextureRGB(nTextureID, x, y, pchData, wide, tall);
}

void ISurface::DrawUpdateRegionTextureRGBA(int nTextureID, int x, int y, const unsigned char *pchData, int wide, int tall)
{
	g_Surface.DrawUpdateRegionTextureRGBA(nTextureID, x, y, pchData, wide, tall);
}

void ISurface::DrawUpdateRegionTextureBGR(int nTextureID, int x, int y, const unsigned char *pchData, int wide, int tall)
{
	g_Surface.DrawUpdateRegionTextureBGR(nTextureID, x, y, pchData, wide, tall);
}

void ISurface::DrawUpdateRegionTextureBGRA(int nTextureID, int x, int y, const unsigned char *pchData, int wide, int tall)
{
	g_Surface.DrawUpdateRegionTextureBGRA(nTextureID, x, y, pchData, wide, tall);
}

void ISurface::DeleteTextureByID(int id)
{
	g_Surface.DeleteTextureByID(id);
}

bool ISurface::AddBitmapFontFile(const char *fontFileName)
{
	return g_Surface.AddBitmapFontFile(fontFileName);
}

void ISurface::SetBitmapFontName(const char *pName, const char *pFontFilename)
{
	return g_Surface.SetBitmapFontName(pName, pFontFilename);
}

const char *ISurface::GetBitmapFontName(const char *pName)
{
	return g_Surface.GetBitmapFontName(pName);
}

//void TestFuckup(void)
//{
//		int i;
//		int _needMouse = false;
//		int _needKB = false;
//
//		for(i = 0 ; i < surface()->GetPopupCount() ; i++ )
//		{
//			VPanel *pop = (VPanel *)surface()->GetPopup( i ) ;
//
//			bool isVisible=pop->IsVisible();
//
//			if (pop)
//			{
//				const char *name = pop->GetName();
//				if (name && strlen(name) > 0)
//					Msg("!!!!!!!!!!TestFuckup:%s isVisible:%d\n", name, isVisible);
//			}
//
//			VPanel *p= pop->GetParent();
//
//			while(p && isVisible)
//			{
//				if( p->IsVisible()==false)
//				{
//					isVisible=false;
//					break;
//				}
//				p=p->GetParent();
//			}
//
//			if (p)
//			{
//				const char *name = p->GetName();
//				if (name && strlen(name) > 0)
//					Msg("TestFuckup:%s isVisible:%d\n", name, isVisible);
//			}
//			if(isVisible)
//			{
//				_needMouse |= pop->IsMouseInputEnabled();
//				_needKB |= pop->IsKeyBoardInputEnabled();
//			}
//		}
//		Msg("TestFuckup:%d %d\n", _needMouse, _needKB);
//}
