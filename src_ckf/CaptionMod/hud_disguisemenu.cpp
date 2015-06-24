#include "hud_base.h"
#include "exportfuncs.h"
#include "client.h"
#include "cvar.h"
#include "util.h"

static xywh_t g_xywhMenu;

static xy_t g_xyFontMenu;
static vgui::HFont g_hFontMenu;

static int g_texIcoMenu;
static xywh_t g_xywhIcoMenu;

static xywh_t g_xywhKey[9];
static wchar_t *g_wszFontKey[] = {L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8", L"9"};

static int g_sClassIndex[] = {1, 3, 4, 8, 2, 7, 6, 5, 9};
static char *g_szClassNick[] = {"sco", "sol", "pyr", "dem", "hvy", "eng", "med", "sni", "spy", "ran"};
static int g_texMan[9][2];
static xywh_t g_xywhMan[9];

static xy_t g_xyBack;

static xy_t g_xySwitchTeam;

static color24 g_ubColor[] = {{236,227,203},{42,39,37}};
static flcol24_t g_flColor[] = {{236/255.0f,227/255.0f,203/255.0f}};

int HudDisguiseMenu_VidInit(void)
{
	for(int i = 0; i < 9; ++i)
	{
		for(int j = 0; j < 2; ++j)
		{
			g_texMan[i][j] = Surface_LoadTGA(va("resource\\tga\\btn_sel_%s_1440_%d", g_szClassNick[i], j+1));
		}
	}
	g_texIcoMenu = Surface_LoadTGA("resource\\tga\\ico_spy");

	g_hFontMenu = g_pSurface->CreateFont();
	g_pSurface->AddGlyphSetToFont(g_hFontMenu, "TF2 Build", 40 * ScreenHeight / 480, 0, 0, 0, vgui::ISurface::FONTFLAG_CUSTOM | vgui::ISurface::FONTFLAG_ANTIALIAS, 0x48, 0x57);

	g_xywhMenu.x = g_xOffset + ScreenHeight * .2;
	g_xywhMenu.y = ScreenHeight * .5;
	g_xywhMenu.w = ScreenHeight * .95;
	g_xywhMenu.h = ScreenHeight * .36;

	g_xyFontMenu.x = g_xywhMenu.x + ScreenHeight * .09;
	g_xyFontMenu.y = g_xywhMenu.y + ScreenHeight * -.033;

	g_xywhIcoMenu.x = g_xywhMenu.x + ScreenHeight * -.008;
	g_xywhIcoMenu.y = g_xywhMenu.y + ScreenHeight * -.056;
	g_xywhIcoMenu.w = ScreenHeight * .145;
	g_xywhIcoMenu.h = ScreenHeight * .145;

	for(int i = 0; i < 3; ++i)
	{
		for(int j = 0; j < 3; ++j)
		{
			g_xywhMan[i*3+j].x = g_xywhMenu.x + ScreenHeight * (.025 + i*.315 + j*.085);
			g_xywhMan[i*3+j].y = g_xywhMenu.y + ScreenHeight * .08;
			g_xywhMan[i*3+j].w = ScreenHeight * .1;
			g_xywhMan[i*3+j].h = ScreenHeight * .1 * 2;

			g_xywhKey[i*3+j].x = g_xywhMenu.x + ScreenHeight * (.06 + i*.315 + j*.085);
			g_xywhKey[i*3+j].y = g_xywhMenu.y + ScreenHeight * .295;
			g_xywhKey[i*3+j].w = ScreenHeight * .04;
			g_xywhKey[i*3+j].h = ScreenHeight * .04;
		}
	}

	g_xyBack.x = g_xywhMenu.x + .80 * ScreenHeight;
	g_xyBack.y = g_xywhMenu.y + .04 * ScreenHeight;

	g_xySwitchTeam.x = g_xywhMenu.x + .1 * ScreenHeight;
	g_xySwitchTeam.y = g_xywhMenu.y + .04 * ScreenHeight;

	return 1;
}

int HudDisguiseMenu_Redraw(float flTime, int iIntermission)
{
	if(g_iHudMenu != HUDMENU_DISGUISE)
		return 0;
	if(g_iWeaponID != WEAPON_DISGUISEKIT)
	{
		g_iHudMenu = 0;
		return 0;
	}
	if(g_iDisgMenuTeam != 1 && g_iDisgMenuTeam != 2)
		return 0;

	g_pSurface->DrawSetColor(255,255,255,255);
	DrawHudMenu(g_xywhMenu.x, g_xywhMenu.y, g_xywhMenu.w, g_xywhMenu.h);

	g_pSurface->DrawSetColor(0,0,0,255);
	g_pSurface->DrawSetTexture(g_texIcoMenu);
	g_pSurface->DrawTexturedRect(g_xywhIcoMenu.x + 2, g_xywhIcoMenu.y + 2, g_xywhIcoMenu.x+g_xywhIcoMenu.w + 2, g_xywhIcoMenu.y+g_xywhIcoMenu.h + 2);

	g_pSurface->DrawSetColor(255,255,255,255);
	g_pSurface->DrawSetTexture(g_texIcoMenu);
	g_pSurface->DrawTexturedRect(g_xywhIcoMenu.x, g_xywhIcoMenu.y, g_xywhIcoMenu.x+g_xywhIcoMenu.w, g_xywhIcoMenu.y+g_xywhIcoMenu.h);

	g_pSurface->DrawSetTextColor(0,0,0,255);
	g_pSurface->DrawSetTextPos(g_xyFontMenu.x+1, g_xyFontMenu.y+1);
	g_pSurface->DrawSetTextFont(g_hFontMenu);
	g_pSurface->DrawPrintText(g_wszDisguise, wcslen(g_wszDisguise));

	g_pSurface->DrawSetTextColor(g_ubColor[0].r,g_ubColor[0].g,g_ubColor[0].b,255);
	g_pSurface->DrawSetTextPos(g_xyFontMenu.x, g_xyFontMenu.y);
	g_pSurface->DrawSetTextFont(g_hFontMenu);
	g_pSurface->DrawPrintText(g_wszDisguise, wcslen(g_wszDisguise));

	for(int i = 0; i < 9; ++i)
	{
		g_pSurface->DrawSetColor(255,255,255,255);
		g_pSurface->DrawSetTexture(g_texMan[i][g_iDisgMenuTeam-1]);
		g_pSurface->DrawTexturedRect(g_xywhMan[i].x, g_xywhMan[i].y, g_xywhMan[i].x+g_xywhMan[i].w, g_xywhMan[i].y+g_xywhMan[i].h);

		//key button
		g_pSurface->DrawSetColor(255,255,255,255);
		DrawHudMenuKey(g_xywhKey[i].x, g_xywhKey[i].y, g_xywhKey[i].w, g_xywhKey[i].h);
		g_pSurface->DrawSetTextColor(g_ubColor[1].r,g_ubColor[1].g,g_ubColor[1].b,255);
		DrawHudMenuKeyLabel(g_wszFontKey[i], g_xywhKey[i].x, g_xywhKey[i].y, g_xywhKey[i].w, g_xywhKey[i].h);
	}

	gEngfuncs.pfnDrawSetTextColor(g_flColor[0].r,g_flColor[0].g,g_flColor[0].b);
	int w, h;
	gEngfuncs.pfnDrawConsoleStringLen(g_szHudMenuBack, &w, &h);
	gEngfuncs.pfnDrawConsoleString(g_xyBack.x - w, g_xyBack.y, g_szHudMenuBack);

	gEngfuncs.pfnDrawSetTextColor(g_flColor[0].r,g_flColor[0].g,g_flColor[0].b);
	gEngfuncs.pfnDrawConsoleString(g_xySwitchTeam.x, g_xySwitchTeam.y, g_szDisgMenuSwitch);
	return 1;
}

int HudDisguiseMenu_KeyEvent(int eventcode, int keynum, const char *pszCurrentBinding)
{
	if(g_iHudMenu != HUDMENU_DISGUISE)
		return 0;
	if(g_iDisgMenuTeam != 1 && g_iDisgMenuTeam != 2)
		return 0;

	char cmd[32];
	if('1' <= keynum && keynum <= '9')
	{
		if(eventcode == 1)
		{
			int i = keynum-'1';
			g_iHudMenu = 0;
			sprintf(cmd, "disguise %d %d\n", g_iDisgMenuTeam, g_sClassIndex[i]);
			gEngfuncs.pfnClientCmd(cmd);
			return 1;
		}
	}
	else if(keynum == 'r')
	{
		if(eventcode == 1)
		{
			g_iDisgMenuTeam = 3-g_iDisgMenuTeam;
		}
		return 1;
	}
	else if(keynum >= '0' && keynum <= '9')
	{
		return 1;
	}
	return 0;
}