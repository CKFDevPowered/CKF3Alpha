#include "hud_base.h"
#include "exportfuncs.h"
#include "client.h"

static int g_texScope;

static int g_texInvulnOverlay[2];

static xywh_t g_xyScope[4];

int HudOverlay_VidInit(void)
{
	if(!g_iHudVidInitalized)
	{
		g_texScope = Surface_LoadTGA("resource\\tga\\scope_1440");
		g_texInvulnOverlay[0] = Surface_LoadTGA("resource\\tga\\overlay_invuln_red");
		g_texInvulnOverlay[1] = Surface_LoadTGA("resource\\tga\\overlay_invuln_red");
	}

	g_xyScope[0].x = ScreenWidth/2 - ScreenHeight*8/9;
	g_xyScope[0].y = 0;
	g_xyScope[0].w = ScreenHeight*8/9;
	g_xyScope[0].h = ScreenHeight/2;

	g_xyScope[1].x = ScreenWidth/2 + ScreenHeight*8/9;
	g_xyScope[1].y = 0;
	g_xyScope[1].w = -g_xyScope[0].w;
	g_xyScope[1].h = ScreenHeight/2;

	g_xyScope[2].x = ScreenWidth/2 + ScreenHeight*8/9;
	g_xyScope[2].y = ScreenHeight;
	g_xyScope[2].w = -g_xyScope[0].w;
	g_xyScope[2].h = -g_xyScope[0].h;

	g_xyScope[3].x = ScreenWidth/2 - ScreenHeight*8/9;
	g_xyScope[3].y = ScreenHeight;
	g_xyScope[3].w = g_xyScope[0].w;
	g_xyScope[3].h = -g_xyScope[0].h;

	return 1;
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

	if(g_pBTEClient->GetHUDFov() < 60)
	{
		DrawScope(flTime);
	}

	return 1;
}