#include "hud_base.h"
#include "exportfuncs.h"
#include "client.h"
#include "cvar.h"
#include "util.h"

static xywh_t g_xywhMenu;

static xywh_t g_xywhIcoMenu;

static xywh_t g_xywhKey[4];
static wchar_t *g_wszFontKey[] = {L"1", L"2", L"3", L"4"};

static xy_t g_xyFontMenu;
static vgui::HFont g_hFontMenu;

static xy_t g_xyBack;

static xy_t g_xyFontBuild[4];

static xywh_t g_xywhBlock[4];
static int g_texEnable;
static int g_texDisable;
static xywh_t g_xywhIcoDemolish[4];
static int g_texIcoDemolish;
static xywh_t g_xywhIcoBuild[4];
static int g_texIcoBuild[4][3];

static xy_t g_xyFontBlock[4];

static color24 g_ubColor[] = {{236,227,203},{42,39,37}};
static flcol24_t g_flColor[] = {{236/255.0f,227/255.0f,203/255.0f}};

int HudDemolishMenu_VidInit(void)
{
	g_texEnable = Surface_LoadTGA("resource\\tga\\hud_demo_enable");
	g_texDisable = Surface_LoadTGA("resource\\tga\\hud_build_disable");
	g_texIcoDemolish = Surface_LoadTGA("resource\\tga\\ico_demolish");
	g_texIcoBuild[0][0] = Surface_LoadTGA("resource\\tga\\ico_sentry1");
	g_texIcoBuild[0][1] = Surface_LoadTGA("resource\\tga\\ico_sentry2");
	g_texIcoBuild[0][2] = Surface_LoadTGA("resource\\tga\\ico_sentry3");
	g_texIcoBuild[1][0] = Surface_LoadTGA("resource\\tga\\ico_dispenser");
	g_texIcoBuild[2][0] = Surface_LoadTGA("resource\\tga\\ico_telein");
	g_texIcoBuild[3][0] = Surface_LoadTGA("resource\\tga\\ico_teleout");

	g_hFontMenu = g_pSurface->CreateFont();
	g_pSurface->AddGlyphSetToFont(g_hFontMenu, "TF2 Build", 40 * ScreenHeight / 480, 0, 0, 0, vgui::ISurface::FONTFLAG_CUSTOM | vgui::ISurface::FONTFLAG_ANTIALIAS, 0x48, 0x57);

	g_xywhMenu.x = g_xOffset + ScreenHeight * .2;
	g_xywhMenu.y = ScreenHeight * .5;
	g_xywhMenu.w = ScreenHeight * .95;
	g_xywhMenu.h = ScreenHeight * .35;

	g_xyFontMenu.x = g_xywhMenu.x + ScreenHeight * .09;
	g_xyFontMenu.y = g_xywhMenu.y + ScreenHeight * -.033;

	g_xywhIcoMenu.x = g_xywhMenu.x + ScreenHeight * .008;
	g_xywhIcoMenu.y = g_xywhMenu.y + ScreenHeight * -.056;
	g_xywhIcoMenu.w = ScreenHeight * .145;
	g_xywhIcoMenu.h = ScreenHeight * .145;

	g_xyBack.x = g_xywhMenu.x + .80 * ScreenHeight;
	g_xyBack.y = g_xywhMenu.y + .01 * ScreenHeight;

	for(int i = 0; i < 4; ++i)
	{
		g_xyFontBuild[i].x = g_xywhMenu.x + (.07 + i*.215) * ScreenHeight;
		g_xyFontBuild[i].y = g_xywhMenu.y + .0725 * ScreenHeight;

		g_xywhBlock[i].x = g_xywhMenu.x + (.06+.215*i) * ScreenHeight;
		g_xywhBlock[i].y = g_xywhMenu.y + .1 * ScreenHeight;
		g_xywhBlock[i].w = .18 * ScreenHeight;
		g_xywhBlock[i].h = .18 * ScreenHeight;

		g_xywhIcoDemolish[i].x = g_xywhMenu.x + (.075 + .215*i) * ScreenHeight;
		g_xywhIcoDemolish[i].y = g_xywhMenu.y + .115 * ScreenHeight;
		g_xywhIcoDemolish[i].w = .15 * ScreenHeight;
		g_xywhIcoDemolish[i].h = .15 * ScreenHeight;

		g_xywhIcoBuild[i].x = g_xywhMenu.x + (.077 + .215*i) * ScreenHeight;
		g_xywhIcoBuild[i].y = g_xywhMenu.y + .115 * ScreenHeight;
		g_xywhIcoBuild[i].w = .15 * ScreenHeight;
		g_xywhIcoBuild[i].h = .15 * ScreenHeight;

		g_xyFontBlock[i].x = g_xywhMenu.x + (.117 + .215*i) * ScreenHeight;
		g_xyFontBlock[i].y = g_xywhMenu.y + .19 * ScreenHeight;

		g_xywhKey[i].x = g_xywhMenu.x + (.13 + .215*i) * ScreenHeight;
		g_xywhKey[i].y = g_xywhMenu.y + .29 * ScreenHeight;
		g_xywhKey[i].w = .04 * ScreenHeight;
		g_xywhKey[i].h = .04 * ScreenHeight;
	}

	return 1;
}

#define CANDESTROY_NOBUILD 0
#define CANDESTROY_YES 1

int GetCanDestroy(int build)
{
	if(build == BUILDABLE_SENTRY)
	{
		if(g_Build.sentry.level)
			return CANDESTROY_YES;
	}
	else if(build == BUILDABLE_DISPENSER)
	{
		if(g_Build.dispenser.level)
			return CANDESTROY_YES;
	}
	else if(build == BUILDABLE_ENTRANCE)
	{
		if(g_Build.telein.level)
			return CANDESTROY_YES;
	}
	else if(build == BUILDABLE_EXIT)
	{
		if(g_Build.teleout.level)
			return CANDESTROY_YES;
	}
	return CANDESTROY_NOBUILD;
}

int HudDemolishMenu_Redraw(float flTime, int iIntermission)
{
	if(g_iHudMenu != HUDMENU_DEMOLISH)
		return 0;
	if(g_iWeaponID != WEAPON_DESTROYPDA)
	{
		//g_iHudMenu = 0;
		return 0;
	}

	static wchar_t wszText[32];
	int i;

	g_pSurface->DrawSetColor(255,255,255,255);
	DrawHudMenu(g_xywhMenu.x, g_xywhMenu.y, g_xywhMenu.w, g_xywhMenu.h);

	g_pSurface->DrawSetColor(g_ubColor[0].r,g_ubColor[0].g,g_ubColor[0].b,255);
	g_pSurface->DrawSetTexture(g_texIcoDemolish);
	g_pSurface->DrawTexturedRect(g_xywhIcoMenu.x, g_xywhIcoMenu.y, g_xywhIcoMenu.x+g_xywhIcoMenu.w, g_xywhIcoMenu.y+g_xywhIcoMenu.h);

	g_pSurface->DrawSetTextColor(0,0,0,255);
	g_pSurface->DrawSetTextPos(g_xyFontMenu.x+1, g_xyFontMenu.y+1);
	g_pSurface->DrawSetTextFont(g_hFontMenu);
	g_pSurface->DrawPrintText(g_wszDemolish, wcslen(g_wszDemolish));

	g_pSurface->DrawSetTextColor(g_ubColor[0].r,g_ubColor[0].g,g_ubColor[0].b,255);
	g_pSurface->DrawSetTextPos(g_xyFontMenu.x, g_xyFontMenu.y);
	g_pSurface->DrawSetTextFont(g_hFontMenu);
	g_pSurface->DrawPrintText(g_wszDemolish, wcslen(g_wszDemolish));

	for(i = 0; i < 4; ++i)
	{
		int canDestroy = GetCanDestroy(i + 1);

		g_pSurface->DrawSetColor(255,255,255,255);
		g_pSurface->DrawSetTexture((canDestroy) ? g_texEnable : g_texDisable);
		g_pSurface->DrawTexturedRect(g_xywhBlock[i].x, g_xywhBlock[i].y, g_xywhBlock[i].x+g_xywhBlock[i].w, g_xywhBlock[i].y+g_xywhBlock[i].h);

		if(canDestroy)
		{
			g_pSurface->DrawSetColor(255,255,255,255);
			g_pSurface->DrawSetTexture(g_texIcoDemolish);
			g_pSurface->DrawTexturedRect(g_xywhIcoDemolish[i].x, g_xywhIcoDemolish[i].y, g_xywhIcoDemolish[i].x+g_xywhIcoDemolish[i].w, g_xywhIcoDemolish[i].y+g_xywhIcoDemolish[i].h);

			int texBuild = 0;
			if(i == 0 && g_Build.sentry.level >= 1 && g_Build.sentry.level <= 3)
			{
				texBuild = g_texIcoBuild[0][g_Build.sentry.level-1];
			}
			else
			{
				texBuild = g_texIcoBuild[i][0];
			}
			if(texBuild)
			{
				g_pSurface->DrawSetTexture(texBuild);
				g_pSurface->DrawTexturedRect(g_xywhIcoBuild[i].x, g_xywhIcoBuild[i].y, g_xywhIcoBuild[i].x+g_xywhIcoBuild[i].w, g_xywhIcoBuild[i].y+g_xywhIcoBuild[i].h);
			}
		}
		else
		{
			gEngfuncs.pfnDrawSetTextColor(g_flColor[0].r,g_flColor[0].g,g_flColor[0].b);
			gEngfuncs.pfnDrawConsoleString(g_xyFontBlock[i].x, g_xyFontBlock[i].y, g_szNotBuilt);
		}
		//build name
		gEngfuncs.pfnDrawSetTextColor(g_flColor[0].r,g_flColor[0].g,g_flColor[0].b);
		gEngfuncs.pfnDrawConsoleString(g_xyFontBuild[i].x, g_xyFontBuild[i].y, g_szBuildables[i]);

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
	return 1;
}

int HudDemolishMenu_KeyEvent(int eventcode, int keynum, const char *pszCurrentBinding)
{
	if(g_iHudMenu != HUDMENU_DEMOLISH)
		return 0;
	char cmd[32];
	if('1' <= keynum && keynum <= '4')
	{
		if(eventcode == 1)
		{
			int i = keynum-'1';
			if(GetCanDestroy(i + 1))
			{
				g_iHudMenu = 0;
				sprintf(cmd, "demolish %d\n", i+1);
				gEngfuncs.pfnClientCmd(cmd);
			}
			return 1;
		}
	}
	else if(keynum == '4')
	{
		return 0;
	}
	else if(keynum >= '0' && keynum <= '9')
	{
		return 1;
	}
	return 0;
}