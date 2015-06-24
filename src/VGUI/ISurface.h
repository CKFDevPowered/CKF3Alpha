#ifndef ISURFACE_H
#define ISURFACE_H

#ifdef _WIN32
#pragma once
#endif

#include <vgui/VGUI.h>
#include <vgui/IHTML.h>
#include "tier1/interface.h"

#ifdef CreateFont
#undef CreateFont
#endif

#ifdef PlaySound
#undef PlaySound
#endif

#ifdef GetCharWidth
#undef GetCharWidth
#endif

class Color;

class IHTMLResponses;
class IHTMLChromeController;

namespace vgui
{

class IImage;
class Image;
class Point;

typedef unsigned long HCursor;
typedef unsigned long HTexture;
typedef unsigned long HFont;

class ISurface : public IBaseInterface
{
public:
	enum EFontFlags
	{
		FONTFLAG_NONE,
		FONTFLAG_ITALIC = 0x001,
		FONTFLAG_UNDERLINE = 0x002,
		FONTFLAG_STRIKEOUT = 0x004,
		FONTFLAG_SYMBOL = 0x008,
		FONTFLAG_ANTIALIAS = 0x010,
		FONTFLAG_GAUSSIANBLUR = 0x020,
		FONTFLAG_ROTARY = 0x040,
		FONTFLAG_DROPSHADOW = 0x080,
		FONTFLAG_ADDITIVE = 0x100,
		FONTFLAG_OUTLINE = 0x200,
		FONTFLAG_CUSTOM = 0x400,
		FONTFLAG_BITMAP = 0x800,
	};

	enum SurfaceFeature_e
	{
		ANTIALIASED_FONTS = 1,
		DROPSHADOW_FONTS = 2,
		ESCAPE_KEY = 3,
		OPENING_NEW_HTML_WINDOWS = 4,
		FRAME_MINIMIZE_MAXIMIZE = 5,
		DIRECT_HWND_RENDER = 6,
		OUTLINE_FONTS = 7,
	};

public:
	virtual void Shutdown(void) = 0;
	virtual void RunFrame(void) = 0;
	virtual VPANEL GetEmbeddedPanel(void) = 0;
	virtual void SetEmbeddedPanel(VPANEL pPanel) = 0;
	virtual void PushMakeCurrent(VPANEL panel, bool useInsets) = 0;
	virtual void PopMakeCurrent(VPANEL panel) = 0;
	virtual void DrawSetColor(int r, int g, int b, int a) = 0;
	virtual void DrawSetColor(Color col) = 0;
	virtual void DrawFilledRect(int x0, int y0, int x1, int y1) = 0;
	virtual void DrawOutlinedRect(int x0, int y0, int x1, int y1) = 0;
	virtual void DrawLine(int x0, int y0, int x1, int y1) = 0;
	virtual void DrawPolyLine(int *px, int *py, int numPoints) = 0;
	virtual void DrawSetTextFont(HFont font) = 0;
	virtual void DrawSetTextColor(int r, int g, int b, int a) = 0;
	virtual void DrawSetTextColor(Color col) = 0;
	virtual void DrawSetTextPos(int x, int y) = 0;
	virtual void DrawGetTextPos(int &x, int &y) = 0;
	virtual void DrawPrintText(const wchar_t *text, int textLen) = 0;
	virtual void DrawUnicodeChar(wchar_t wch) = 0;
	virtual void DrawUnicodeCharAdd(wchar_t wch) = 0;
	virtual void DrawFlushText(void) = 0;
	virtual IHTML *CreateHTMLWindow(IHTMLEvents *events, VPANEL context) = 0;
	virtual void PaintHTMLWindow(IHTML *htmlwin) = 0;
	virtual void DeleteHTMLWindow(IHTML *htmlwin) = 0;
	virtual void DrawSetTextureFile(int id, const char *filename, int hardwareFilter, bool forceReload) = 0;
	virtual void DrawSetTextureRGBA(int id, const unsigned char *rgba, int wide, int tall, int hardwareFilter, bool forceReload) = 0;
	virtual void DrawSetTexture(int id) = 0;
	virtual void DrawGetTextureSize(int id, int &wide, int &tall) = 0;
	virtual void DrawTexturedRect(int x0, int y0, int x1, int y1) = 0;
	virtual bool IsTextureIDValid(int id) = 0;
	virtual int CreateNewTextureID(bool procedural = false) = 0;
	virtual void GetScreenSize(int &wide, int &tall) = 0;
	virtual void SetAsTopMost(VPANEL panel, bool state) = 0;
	virtual void BringToFront(VPANEL panel) = 0;
	virtual void SetForegroundWindow(VPANEL panel) = 0;
	virtual void SetPanelVisible(VPANEL panel, bool state) = 0;
	virtual void SetMinimized(VPANEL panel, bool state) = 0;
	virtual bool IsMinimized(VPANEL panel) = 0;
	virtual void FlashWindow(VPANEL panel, bool state) = 0;
	virtual void SetTitle(VPANEL panel, const wchar_t *title) = 0;
	virtual void SetAsToolBar(VPANEL panel, bool state) = 0;
	virtual void CreatePopup(VPANEL panel, bool minimised, bool showTaskbarIcon = true, bool disabled = false, bool mouseInput = true, bool kbInput = true) = 0;
	virtual void SwapBuffers(VPANEL panel) = 0;
	virtual void Invalidate(VPANEL panel) = 0;
	virtual void SetCursor(HCursor cursor) = 0;
	virtual bool IsCursorVisible(void) = 0;
	virtual void ApplyChanges(void) = 0;
	virtual bool IsWithin(int x, int y) = 0;
	virtual bool HasFocus(void) = 0;
	virtual bool SupportsFeature(SurfaceFeature_e feature) = 0;
	virtual void RestrictPaintToSinglePanel(VPANEL panel) = 0;
	virtual void SetModalPanel(VPANEL panel) = 0;
	virtual VPANEL GetModalPanel(void) = 0;
	virtual void UnlockCursor(void) = 0;
	virtual void LockCursor(void) = 0;
	virtual void SetTranslateExtendedKeys(bool state) = 0;
	virtual VPANEL GetTopmostPopup(void) = 0;
	virtual void SetTopLevelFocus(VPANEL panel) = 0;
	virtual HFont CreateFont(void) = 0;
	virtual bool AddGlyphSetToFont(HFont font, const char *windowsFontName, int tall, int weight, int blur, int scanlines, int flags, int lowRange, int highRange) = 0;
	virtual bool AddCustomFontFile(const char *fontFileName) = 0;
	virtual int GetFontTall(HFont font) = 0;
	virtual void GetCharABCwide(HFont font, int ch, int &a, int &b, int &c) = 0;
	virtual int GetCharacterWidth(HFont font, int ch) = 0;
	virtual void GetTextSize(HFont font, const wchar_t *text, int &wide, int &tall) = 0;
	virtual VPANEL GetNotifyPanel(void) = 0;
	virtual void SetNotifyIcon(VPANEL context, HTexture icon, VPANEL panelToReceiveMessages, const char *text) = 0;
	virtual void PlaySound(const char *fileName) = 0;
	virtual int GetPopupCount(void) = 0;
	virtual VPANEL GetPopup(int index) = 0;
	virtual bool ShouldPaintChildPanel(VPANEL childPanel) = 0;
	virtual bool RecreateContext(VPANEL panel) = 0;
	virtual void AddPanel(VPANEL panel) = 0;
	virtual void ReleasePanel(VPANEL panel) = 0;
	virtual void MovePopupToFront(VPANEL panel) = 0;
	virtual void MovePopupToBack(VPANEL panel) = 0;
	virtual void SolveTraverse(VPANEL panel, bool forceApplySchemeSettings = false) = 0;
	virtual void PaintTraverse(VPANEL panel) = 0;
	virtual void EnableMouseCapture(VPANEL panel, bool state) = 0;
	virtual void GetWorkspaceBounds(int &x, int &y, int &wide, int &tall) = 0;
	virtual void GetAbsoluteWindowBounds(int &x, int &y, int &wide, int &tall) = 0;
	virtual void GetProportionalBase(int &width, int &height) = 0;
	virtual void CalculateMouseVisible(void) = 0;
	virtual bool NeedKBInput(void) = 0;
	virtual bool HasCursorPosFunctions(void) = 0;
	virtual void SurfaceGetCursorPos(int &x, int &y) = 0;
	virtual void SurfaceSetCursorPos(int x, int y) = 0;
	virtual void DrawTexturedPolygon(int *p, int n) = 0;
	virtual int GetFontAscent(HFont font, wchar_t wch) = 0;
	virtual void SetAllowHTMLJavaScript(bool state) = 0;

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
};
}

#define VGUI_SURFACE_INTERFACE_VERSION "VGUI_Surface026"

#endif