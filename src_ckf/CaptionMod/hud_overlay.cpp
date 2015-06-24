#include "hud_base.h"
#include "exportfuncs.h"
#include "client.h"

static int g_texScope;

typedef struct disguise_hint_s
{
public:
	int tex;
	int show;
	xywh_t start;
	xywh_t end;
	float animtime;
}disguise_hint_t;

static disguise_hint_t g_DisguiseHint;

static xywh_t g_xyScope[4];

int HudOverlay_VidInit(void)
{
	g_texScope = Surface_LoadTGA("resource\\tga\\scope_1440");
	g_DisguiseHint.tex = Surface_LoadTGA("resource\\tga\\disguise_hint");

	g_DisguiseHint.start.x = ScreenHeight * (30 / 600.0);
	g_DisguiseHint.start.y = ScreenHeight * (525 / 600.0);
	g_DisguiseHint.start.w = ScreenHeight * (75 / 600.0);
	g_DisguiseHint.start.h = g_DisguiseHint.start.w;

	g_DisguiseHint.end.w = ScreenHeight * (508 / 600.0);
	g_DisguiseHint.end.h = g_DisguiseHint.end.w;
	g_DisguiseHint.end.x = (ScreenWidth - g_DisguiseHint.end.w) / 2;
	g_DisguiseHint.end.y = ScreenHeight * (48 / 600.0);

	g_xyScope[0].x = g_bgOffset;
	g_xyScope[0].y = 0;
	g_xyScope[0].w = ScreenHeight*8/9;
	g_xyScope[0].h = ScreenHeight/2;
	g_xyScope[1].x = ScreenWidth-g_bgOffset;
	g_xyScope[1].y = 0;
	g_xyScope[1].w = -g_xyScope[0].w;
	g_xyScope[1].h = ScreenHeight/2;
	g_xyScope[2].x = ScreenWidth-g_bgOffset;
	g_xyScope[2].y = ScreenHeight;
	g_xyScope[2].w = -g_xyScope[0].w;
	g_xyScope[2].h = -g_xyScope[0].h;
	g_xyScope[3].x = g_bgOffset;
	g_xyScope[3].y = ScreenHeight;
	g_xyScope[3].w = g_xyScope[0].w;
	g_xyScope[3].h = -g_xyScope[0].h;

	return 1;
}

void CL_DisguiseHint(void)
{
	g_DisguiseHint.show = 1;
	g_DisguiseHint.animtime = g_flClientTime;
}

void DrawDisguiseHint(void)
{
	if(!g_DisguiseHint.show)
		return;

	int x, y, w, h;
	if(g_DisguiseHint.show == 1)
	{
		float frac = (g_flClientTime - g_DisguiseHint.animtime) / 0.25f;
		if(frac > 1.0f)
		{
			frac = 1;
			g_DisguiseHint.show = 2;
			g_DisguiseHint.animtime = g_flClientTime;
		}
		float frac2 = 1 - frac;

		x = g_DisguiseHint.start.x * frac2 + g_DisguiseHint.end.x * frac;
		y = g_DisguiseHint.start.y * frac2 + g_DisguiseHint.end.y * frac;
		w = g_DisguiseHint.start.w * frac2 + g_DisguiseHint.end.w * frac;
		h = g_DisguiseHint.start.h * frac2 + g_DisguiseHint.end.h * frac;
	}
	else if(g_DisguiseHint.show == 2)
	{
		if(g_flClientTime - g_DisguiseHint.animtime > 0.75)
		{
			g_DisguiseHint.show = 3;
			g_DisguiseHint.animtime = g_flClientTime;
		}
		x = g_DisguiseHint.end.x;
		y = g_DisguiseHint.end.y;
		w = g_DisguiseHint.end.w;
		h = g_DisguiseHint.end.h;
	}
	else if(g_DisguiseHint.show == 3)
	{
		float frac = (g_flClientTime - g_DisguiseHint.animtime) / 0.25f;
		if(frac > 1.0f)
		{
			frac = 1;
			g_DisguiseHint.show = 0;
			g_DisguiseHint.animtime = g_flClientTime;
		}
		float frac2 = 1 - frac;
		x = g_DisguiseHint.end.x * frac2 + g_DisguiseHint.start.x * frac;
		y = g_DisguiseHint.end.y * frac2 + g_DisguiseHint.start.y * frac;
		w = g_DisguiseHint.end.w * frac2 + g_DisguiseHint.start.w * frac;
		h = g_DisguiseHint.end.h * frac2 + g_DisguiseHint.start.h * frac;
	}
	qglEnable(GL_BLEND);

	g_pSurface->DrawSetColor(255,255,255,255);
	g_pSurface->DrawSetTexture(g_DisguiseHint.tex);

	g_pSurface->DrawTexturedRect(x, y, x+w, y+h);
}

void DrawScope(float flTime)
{
	for(int i = 0; i < 4; ++i)
	{
		g_pSurface->DrawSetColor(255,255,255,255);
		g_pSurface->DrawSetTexture(g_texScope);
		g_pSurface->DrawTexturedRect(g_xyScope[i].x, g_xyScope[i].y, g_xyScope[i].x+g_xyScope[i].w, g_xyScope[i].y+g_xyScope[i].h);
	}
}

int HudOverlay_Redraw(float flTime, int iIntermission)
{
	if(iIntermission)
		return 0;

	if(!CL_IsAlive())
		return 0;

	if(gBTEFuncs.GetHUDFov() < 60)
	{
		DrawScope(flTime);
	}

	if(g_iClass == CLASS_SPY)
	{
		DrawDisguiseHint();
	}

	return 1;
}