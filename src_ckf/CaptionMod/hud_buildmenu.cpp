#include "hud_base.h"
#include "exportfuncs.h"
#include "client.h"
#include "cvar.h"
#include "util.h"
#include "weapon.h"

static xywh_t g_xywhMenu;

static int g_texIcoMenu;
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
static xywh_t g_xywhIcoMetal[4];
static int g_texIcoMetal;
static xywh_t g_xywhBluePrint[4];
static int g_texBluePrint[4];

static vgui::HFont g_hFontMetal;
static xy_t g_xyFontMetal[4];

static xy_t g_xyFontBlock[4];

static int g_iMetalNeed[4] = {135, 100, 125, 125};

static color24 g_ubColor[] = {{236,227,203},{193,21,0},{108,107,94},{42,39,37}};
static flcol24_t g_flColor[] = {{236/255.0f,227/255.0f,203/255.0f},{193/255.0f,21/255.0f,0},{42/255.0f,39/255.0f,37/255.0f}};

int HudBuildMenu_VidInit(void)
{
	g_texIcoMenu = Surface_LoadTGA("resource\\tga\\ico_build");
	g_texEnable = Surface_LoadTGA("resource\\tga\\hud_build_enable");
	g_texDisable = Surface_LoadTGA("resource\\tga\\hud_build_disable");
	g_texIcoMetal = Surface_LoadTGA("resource\\tga\\ico_metal");
	g_texBluePrint[0] = Surface_LoadTGA("resource\\tga\\hud_build_sentry_bp");
	g_texBluePrint[1] = Surface_LoadTGA("resource\\tga\\hud_build_dispenser_bp");
	g_texBluePrint[2] = Surface_LoadTGA("resource\\tga\\hud_build_entrance_bp");
	g_texBluePrint[3] = Surface_LoadTGA("resource\\tga\\hud_build_exit_bp");

	g_hFontMetal = g_pSurface->CreateFont();
	g_pSurface->AddGlyphSetToFont(g_hFontMetal, "TF2", 12 * ScreenHeight / 480, 0, 0, 0, vgui::ISurface::FONTFLAG_CUSTOM | vgui::ISurface::FONTFLAG_ANTIALIAS, 0x48, 0x57);

	g_hFontMenu = g_pSurface->CreateFont();
	g_pSurface->AddGlyphSetToFont(g_hFontMenu, "TF2 Build", 40 * ScreenHeight / 480, 0, 0, 0, vgui::ISurface::FONTFLAG_CUSTOM | vgui::ISurface::FONTFLAG_ANTIALIAS, 0x48, 0x57);

	g_xywhMenu.x = g_xOffset + ScreenHeight * .2;
	g_xywhMenu.y = ScreenHeight * .5;
	g_xywhMenu.w = ScreenHeight * .95;
	g_xywhMenu.h = ScreenHeight * .35;

	g_xywhIcoMenu.x = g_xywhMenu.x + ScreenHeight * .018;
	g_xywhIcoMenu.y = g_xywhMenu.y + ScreenHeight * -.06;
	g_xywhIcoMenu.w = ScreenHeight * .13;
	g_xywhIcoMenu.h = ScreenHeight * .13;

	g_xyFontMenu.x = g_xywhMenu.x + ScreenHeight * .145;
	g_xyFontMenu.y = g_xywhMenu.y + ScreenHeight * -.033;

	g_xyBack.x = g_xywhMenu.x + g_xywhMenu.w - 0.05 * ScreenHeight;
	g_xyBack.y = g_xywhMenu.y + .01 * ScreenHeight;

	for(int i = 0; i < 4; ++i)
	{
		g_xyFontBuild[i].x = g_xywhMenu.x + (.07 + i*.215) * ScreenHeight;
		g_xyFontBuild[i].y = g_xywhMenu.y + .07 * ScreenHeight;

		g_xywhBlock[i].x = g_xywhMenu.x + (.06+.215*i) * ScreenHeight;
		g_xywhBlock[i].y = g_xywhMenu.y + .1 * ScreenHeight;
		g_xywhBlock[i].w = .18 * ScreenHeight;
		g_xywhBlock[i].h = .18 * ScreenHeight;

		g_xywhIcoMetal[i].x = g_xywhMenu.x + (.07 + .215*i) * ScreenHeight;
		g_xywhIcoMetal[i].y = g_xywhMenu.y + .105 * ScreenHeight;
		g_xywhIcoMetal[i].w = .0275 * ScreenHeight;
		g_xywhIcoMetal[i].h = .0275 * ScreenHeight;

		g_xywhBluePrint[i].x = g_xywhMenu.x + (.085 + .215*i) * ScreenHeight;
		g_xywhBluePrint[i].y = g_xywhMenu.y + .135 * ScreenHeight;
		g_xywhBluePrint[i].w = .13 * ScreenHeight;
		g_xywhBluePrint[i].h = .13 * ScreenHeight;

		g_xyFontMetal[i].x = g_xywhMenu.x + (.105 + .215*i) * ScreenHeight;
		g_xyFontMetal[i].y = g_xywhMenu.y + .105 * ScreenHeight;

		g_xyFontBlock[i].x = g_xywhMenu.x + (.117 + .215*i) * ScreenHeight;
		g_xyFontBlock[i].y = g_xywhMenu.y + .19 * ScreenHeight;

		g_xywhKey[i].x = g_xywhMenu.x + (.13 + .215*i) * ScreenHeight;
		g_xywhKey[i].y = g_xywhMenu.y + .29 * ScreenHeight;
		g_xywhKey[i].w = .04 * ScreenHeight;
		g_xywhKey[i].h = .04 * ScreenHeight;
	}
	
	return 1;
}

#define CANBUILD_YES 0
#define CANBUILD_NOMETAL 1
#define CANBUILD_ALREADY 2

int GetCanBuild(int build)
{
	if(build == BUILDABLE_SENTRY)
	{
		if(g_Build.sentry.level)
			return CANBUILD_ALREADY;
		if(g_Player.m_iMetal < g_iMetalNeed[build-1])
			return CANBUILD_NOMETAL;
	}
	else if(build == BUILDABLE_DISPENSER)
	{
		if(g_Build.dispenser.level)
			return CANBUILD_ALREADY;
		if(g_Player.m_iMetal < g_iMetalNeed[build-1])
			return CANBUILD_NOMETAL;
	}
	else if(build == BUILDABLE_ENTRANCE)
	{
		if(g_Build.telein.level)
			return CANBUILD_ALREADY;
		if(g_Player.m_iMetal < g_iMetalNeed[build-1])
			return CANBUILD_NOMETAL;
	}
	else if(build == BUILDABLE_EXIT)
	{
		if(g_Build.teleout.level)
			return CANBUILD_ALREADY;
		if(g_Player.m_iMetal < g_iMetalNeed[build-1])
			return CANBUILD_NOMETAL;
	}
	return CANBUILD_YES;
}

int HudBuildMenu_Redraw(float flTime, int iIntermission)
{
	if(g_iHudMenu != HUDMENU_BUILD)
		return 0;
	if(g_iWeaponID != WEAPON_BUILDPDA)
	{
		g_iHudMenu = 0;
		return 0;
	}

	static wchar_t wszText[32];
	int i;
	int col;

	g_pSurface->DrawSetColor(255,255,255,255);
	DrawHudMenu(g_xywhMenu.x, g_xywhMenu.y, g_xywhMenu.w, g_xywhMenu.h);

	g_pSurface->DrawSetColor(0,0,0,255);
	g_pSurface->DrawSetTexture(g_texIcoMenu);
	g_pSurface->DrawTexturedRect(g_xywhIcoMenu.x + 2, g_xywhIcoMenu.y + 2, g_xywhIcoMenu.x+g_xywhIcoMenu.w + 2, g_xywhIcoMenu.y+g_xywhIcoMenu.h + 2);

	g_pSurface->DrawSetColor(g_ubColor[0].r,g_ubColor[0].g,g_ubColor[0].b,255);
	g_pSurface->DrawSetTexture(g_texIcoMenu);
	g_pSurface->DrawTexturedRect(g_xywhIcoMenu.x, g_xywhIcoMenu.y, g_xywhIcoMenu.x+g_xywhIcoMenu.w, g_xywhIcoMenu.y+g_xywhIcoMenu.h);

	g_pSurface->DrawSetTextColor(0,0,0,255);
	g_pSurface->DrawSetTextPos(g_xyFontMenu.x+1, g_xyFontMenu.y+1);
	g_pSurface->DrawSetTextFont(g_hFontMenu);
	g_pSurface->DrawPrintText(g_wszBuild, wcslen(g_wszBuild));

	g_pSurface->DrawSetTextColor(g_ubColor[0].r,g_ubColor[0].g,g_ubColor[0].b,255);
	g_pSurface->DrawSetTextPos(g_xyFontMenu.x, g_xyFontMenu.y);
	g_pSurface->DrawSetTextFont(g_hFontMenu);
	g_pSurface->DrawPrintText(g_wszBuild, wcslen(g_wszBuild));

	for(i = 0; i < 4; ++i)
	{
		int canBuild = GetCanBuild(i + 1);

		g_pSurface->DrawSetColor(255,255,255,255);
		g_pSurface->DrawSetTexture((canBuild == CANBUILD_YES) ? g_texEnable : g_texDisable);
		g_pSurface->DrawTexturedRect(g_xywhBlock[i].x, g_xywhBlock[i].y, g_xywhBlock[i].x+g_xywhBlock[i].w, g_xywhBlock[i].y+g_xywhBlock[i].h);

		if(canBuild == CANBUILD_ALREADY)//already built
		{
			col = 2;
			gEngfuncs.pfnDrawSetTextColor(g_flColor[0].r,g_flColor[0].g,g_flColor[0].b);
			gEngfuncs.pfnDrawConsoleString(g_xyFontBlock[i].x, g_xyFontBlock[i].y, g_szAlreadyBuilt);
		}
		else if(canBuild == CANBUILD_NOMETAL)//no metal
		{
			col = 1;
			gEngfuncs.pfnDrawSetTextColor(g_flColor[1].r,g_flColor[1].g,g_flColor[1].b);
			gEngfuncs.pfnDrawConsoleString(g_xyFontBlock[i].x, g_xyFontBlock[i].y, g_szNeedMetal);
		}
		else//can build
		{
			col = 3;
			g_pSurface->DrawSetColor(255,255,255,255);
			g_pSurface->DrawSetTexture(g_texBluePrint[i]);
			g_pSurface->DrawTexturedRect(g_xywhBluePrint[i].x, g_xywhBluePrint[i].y, g_xywhBluePrint[i].x+g_xywhBluePrint[i].w, g_xywhBluePrint[i].y+g_xywhBluePrint[i].h);
		}
		//metal ico
		g_pSurface->DrawSetColor(g_ubColor[col].r,g_ubColor[col].g,g_ubColor[col].b,255);
		g_pSurface->DrawSetTexture(g_texIcoMetal);
		g_pSurface->DrawTexturedRect(g_xywhIcoMetal[i].x, g_xywhIcoMetal[i].y, g_xywhIcoMetal[i].x+g_xywhIcoMetal[i].w, g_xywhIcoMetal[i].y+g_xywhIcoMetal[i].h);

		//metal num
		g_pSurface->DrawSetTextColor(g_ubColor[col].r,g_ubColor[col].g,g_ubColor[col].b,255);
		g_pSurface->DrawSetTextPos(g_xyFontMetal[i].x, g_xyFontMetal[i].y);
		g_pSurface->DrawSetTextFont(g_hFontMetal);
		wsprintfW(wszText, L"%d", g_iMetalNeed[i]);
		g_pSurface->DrawPrintText(wszText, wcslen(wszText));

		//build name
		gEngfuncs.pfnDrawSetTextColor(g_flColor[0].r,g_flColor[0].g,g_flColor[0].b);
		gEngfuncs.pfnDrawConsoleString(g_xyFontBuild[i].x, g_xyFontBuild[i].y, g_szBuildables[i]);

		//key button
		g_pSurface->DrawSetColor(255,255,255,255);
		DrawHudMenuKey(g_xywhKey[i].x, g_xywhKey[i].y, g_xywhKey[i].w, g_xywhKey[i].h);
		g_pSurface->DrawSetTextColor(g_ubColor[3].r,g_ubColor[3].g,g_ubColor[3].b,255);
		DrawHudMenuKeyLabel(g_wszFontKey[i], g_xywhKey[i].x, g_xywhKey[i].y, g_xywhKey[i].w, g_xywhKey[i].h);
	}

	g_pSurface->DrawFlushText();

	gEngfuncs.pfnDrawSetTextColor(g_flColor[0].r,g_flColor[0].g,g_flColor[0].b);
	int w, h;
	gEngfuncs.pfnDrawConsoleStringLen(g_szHudMenuBack, &w, &h);
	gEngfuncs.pfnDrawConsoleString(g_xyBack.x - w, g_xyBack.y, g_szHudMenuBack);
	return 1;
}

int HudBuildMenu_KeyEvent(int eventcode, int keynum, const char *pszCurrentBinding)
{
	if(g_iHudMenu != HUDMENU_BUILD)
		return 0;
	char cmd[32];
	if('1' <= keynum && keynum <= '4')
	{
		if(eventcode == 1)
		{
			int i = keynum-'1';
			if(GetCanBuild(i + 1) == CANBUILD_YES)
			{
				g_iHudMenu = 0;
				sprintf(cmd, "build %d\n", i+1);
				gEngfuncs.pfnClientCmd(cmd);
			}
			return 1;
		}
	}
	else if(keynum == '5')
	{
		return 0;
	}
	else if(keynum >= '0' && keynum <= '9')
	{
		return 1;
	}
	return 0;
}