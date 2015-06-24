#pragma once

#include <metahook.h>
#include "engfuncs.h"
#include "surface.h"
#include "localize.h"
#include "qgl.h"
#include "hud_localize.h"

extern SCREENINFO g_ScreenInfo;
extern bool g_bGameUIActivate;

#undef ScreenWidth
#define ScreenWidth (g_ScreenInfo.iWidth)

#undef ScreenHeight
#define ScreenHeight (g_ScreenInfo.iHeight)

typedef struct
{
	float r, g, b;
}flcol24_t;

typedef struct
{
	int x, y, w, h;
}xywh_t;

typedef struct
{
	int x, y;
}xy_t;

typedef struct
{
	int w, h;
}wh_t;

/*class CBaseHud
{
public:
	virtual void Init(void){}
	virtual void VidInit(void){}
	virtual void Redraw(int intermission){}
	virtual void MouseUp(int x, int y){}
	virtual void MouseDown(int x, int y){}
	virtual bool IsFullScreen(void){return false;}
};

class CHudHealth : public CBaseHud
{
public:
	virtual void Init(void);
	virtual void VidInit(void);
	virtual void Redraw(int intermission);
	void DrawCross(int x, int y, float health, float scale);
public:
	int m_texBg;
	int m_texColor;
	vgui::HFont m_hFont;
};*/

extern int g_bgOffset;
extern int g_xOffset;
extern int g_mouse_state;
extern int g_mouse_oldstate;

int Surface_LoadTGA(const char *filename);
int HudBase_DrawMultiline(const wchar_t *text, int x, int y);
void HudBase_DrawMultilineSetup(int yOffset, int limitWidth);
void HudBase_GetPlayerInfo(void);
bool HudBase_IsMouseInRect(int mx, int my, int x, int y, int w, int h);
void HudBase_MouseUp(int mx, int my);
void HudBase_MouseDown(int mx, int my);
bool HudBase_IsFullScreenMenu(void);
void HudBase_ActivateMouse(void);
void HudBase_DeactivateMouse(void);

void DrawButton(int x, int y, int w, int h);
void DrawButtonEx(int x, int y, int w, int h, int edge);
void DrawButtonLabel(wchar_t *label, int x, int y, int w, int h);
void DrawHudMenu(int x, int y, int w, int h);
void DrawPointsQuad(xy_t *p);
void DrawHudMenuKey(int x, int y, int w, int h);
void DrawHudMenuKeyLabel(wchar_t *label, int x, int y, int w, int h);

inline void AngleVector3DHUD(float *forward, float *right, float *up) {forward[0] = 1;forward[1] = 0;forward[2] = 0;right[0] = 0;right[1] = -1;right[2] = 0;up[0] = 0;up[1] = 0;up[2] = 1;}