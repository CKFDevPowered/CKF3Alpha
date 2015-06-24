#include "hud_base.h"
#include "exportfuncs.h"
#include "client.h"
#include "cvar.h"
#include "StudioModelRenderer.h"
#include "util.h"
#include "cJSON.h"
#include <string>
#include <list>
#include <math.h>

static int g_texMenu;
static vgui::HFont g_hIntroFont;
static vgui::HFont g_hTitleFont;

static xywh_t g_xywhMenu;
static xywh_t g_xywhNext;
static xywh_t g_xywhPrev;
static xy_t g_xyIntroFont;
static xy_t g_xyTitleFont;
static int g_lineOffset;

typedef struct
{
	std::wstring title;
	std::wstring text;
	std::string buf;
}motd_t;

typedef struct
{
	xy_t p[4];
	int tex;
	char path[64];
}mapimage_t;

typedef struct
{
	std::wstring name;
	std::wstring mode;
	std::wstring brief;
	std::list<mapimage_t> image;
	std::string buf;
	int hasinfo;
}mapinfo_t;

static motd_t g_Motd;
static mapinfo_t g_MapInfo;

static int g_fHasMapInfo;
static int g_iPage;

void Motd_Init(void);
void MapInfo_Init(void);

void HudIntroMenu_Init(void)
{
	g_iPage = 0;
	Motd_Init();
	MapInfo_Init();
}

int HudIntroMenu_VidInit(void)
{
	g_texMenu = Surface_LoadTGA("resource\\tga\\ui_intro");

	g_xywhMenu.x = g_bgOffset;
	g_xywhMenu.y = 0;
	g_xywhMenu.w = 16*ScreenHeight/9;
	g_xywhMenu.h = ScreenHeight;

	g_xywhNext.x = ScreenWidth - ScreenHeight * .4;
	g_xywhNext.y = ScreenHeight *.915;
	g_xywhNext.w = ScreenHeight *.3;
	g_xywhNext.h = ScreenHeight *.07;

	g_xywhPrev.x = ScreenHeight * .1;
	g_xywhPrev.y = ScreenHeight *.915;
	g_xywhPrev.w = ScreenHeight *.3;
	g_xywhPrev.h = ScreenHeight *.07;

	g_xyTitleFont.x = g_xOffset + ScreenHeight * .3f;
	g_xyTitleFont.y = ScreenHeight * .2f;
	g_xyIntroFont.x = g_xOffset + ScreenHeight * .3f;
	g_xyIntroFont.y = ScreenHeight * .3f;

	g_lineOffset = max(3, 2 * ScreenHeight / 480);

	g_hIntroFont = g_pSurface->CreateFont();
	g_hTitleFont = g_pSurface->CreateFont();
	g_pSurface->AddGlyphSetToFont(g_hIntroFont, "TF2", 16 * ScreenHeight / 480, 0, 0, 0, vgui::ISurface::FONTFLAG_CUSTOM | vgui::ISurface::FONTFLAG_ANTIALIAS, 0x48, 0x57);
	g_pSurface->AddGlyphSetToFont(g_hTitleFont, "TF2", 32 * ScreenHeight / 480, 0, 0, 0, vgui::ISurface::FONTFLAG_CUSTOM | vgui::ISurface::FONTFLAG_ANTIALIAS, 0x48, 0x57);

	return 1;
}

void HudIntroMenu_SetPage(int page)
{
	if(g_iPage == 0 && page == 1 && !g_MapInfo.hasinfo)
	{
		gEngfuncs.pfnClientCmd("showmapinfo\n");
		g_MapInfo.hasinfo = 1;
	}
	g_iPage = page;
}

void Motd_ParseBuf(void)
{
	cJSON *root = cJSON_Parse(g_Motd.buf.c_str());
	if (!root)
		return;
	cJSON *title = cJSON_GetObjectItem(root, "title");
	if(title && title->valuestring)
	{
		g_Motd.title = UTF8ToUnicode(title->valuestring);
	}
	cJSON *text = cJSON_GetObjectItem(root, "text");
	if(text && text->valuestring)
	{
		g_Motd.text = UTF8ToUnicode(text->valuestring);
	}
	cJSON_Delete(root);
	g_Motd.buf.clear();
}

void Motd_LoadFromFile(void)
{

}

void Motd_ClearBuf(void)
{
	g_Motd.buf.clear();
}

void Motd_AddToBuf(const char *str)
{
	g_Motd.buf.append(str);
}

void Motd_Init(void)
{
	g_Motd.title.clear();
	g_Motd.text.clear();
	g_Motd.buf.clear();
}

void MapInfo_ParseBuf(void)
{
	cJSON *root = cJSON_Parse(g_MapInfo.buf.c_str());
	if (!root)
	{
		gEngfuncs.pfnConsolePrint(va("mapinfo json failed for parsing.\n"));
		return;
	}
	cJSON *name = cJSON_GetObjectItem(root, "name");
	if(name && name->valuestring)
	{
		g_MapInfo.name = UTF8ToUnicode(name->valuestring);
	}
	cJSON *mode = cJSON_GetObjectItem(root, "mode");
	if(mode && mode->valuestring)
	{
		g_MapInfo.mode = UTF8ToUnicode(mode->valuestring);
	}
	cJSON *brief = cJSON_GetObjectItem(root, "brief");
	if(brief && brief->valuestring)
	{
		g_MapInfo.brief = UTF8ToUnicode(brief->valuestring);
	}
	cJSON *imagelist = cJSON_GetObjectItem(root, "images");
	if(imagelist)
	{
		int num = cJSON_GetArraySize(imagelist);
		for(int i = 0; i < num; ++i)
		{
			cJSON *imageobj = cJSON_GetArrayItem(imagelist, i);
			if(!imageobj) continue;
			mapimage_t mapimage;
			mapimage.tex = 0;
			mapimage.path[0] = 0;
			int xpos, ypos, width, height;
			cJSON *x = cJSON_GetObjectItem(imageobj, "x");
			if(x && cJSON_IsNumber(x->valuestring))
			{
				xpos = g_xOffset + ScreenHeight * atof(x->valuestring);
			}
			cJSON *y = cJSON_GetObjectItem(imageobj, "y");
			if(y && cJSON_IsNumber(y->valuestring))
			{
				ypos = ScreenHeight * atof(y->valuestring);
			}
			cJSON *w = cJSON_GetObjectItem(imageobj, "w");
			if(w && cJSON_IsNumber(w->valuestring))
			{
				width = ScreenHeight * atof(w->valuestring);
			}
			cJSON *h = cJSON_GetObjectItem(imageobj, "h");
			if(h && cJSON_IsNumber(h->valuestring))
			{
				height = ScreenHeight * atof(h->valuestring);
			}
			cJSON *r = cJSON_GetObjectItem(imageobj, "r");
			cJSON *path = cJSON_GetObjectItem(imageobj, "path");
			if(path && path->valuestring)
			{
				strncpy(mapimage.path, path->valuestring, 63);
				mapimage.path[63] = 0;
			}
			mapimage.tex = Surface_LoadTGA(mapimage.path);
			if(r && cJSON_IsNumber(r->valuestring))
			{
				xy_t v1, v2;
				double a = (double)atof(r->valuestring)*M_PI/180;
				v1.x = width*cos(a);
				v1.y = width*sin(a);
				v2.x = -height*sin(a);
				v2.y = height*cos(a);
				mapimage.p[0].x = xpos;
				mapimage.p[0].y = ypos;
				mapimage.p[1].x = mapimage.p[0].x + v1.x;
				mapimage.p[1].y = mapimage.p[0].y + v1.y;
				mapimage.p[2].x = mapimage.p[1].x + v2.x;
				mapimage.p[2].y = mapimage.p[1].y + v2.y;
				mapimage.p[3].x = mapimage.p[2].x - v1.x;
				mapimage.p[3].y = mapimage.p[2].y - v1.y;
			}
			else
			{
				mapimage.p[0].x = xpos;
				mapimage.p[0].y = ypos;
				mapimage.p[1].x = xpos + width;
				mapimage.p[1].y = ypos;
				mapimage.p[2].x = xpos + width;
				mapimage.p[2].y = ypos + height;
				mapimage.p[3].x = xpos;
				mapimage.p[3].y = ypos + height;
			}
			g_MapInfo.image.push_back(mapimage);
		}
	}
	cJSON_Delete(root);
	g_MapInfo.buf.clear();
	g_MapInfo.hasinfo = 1;
}

void MapInfo_LoadFromFile(void)
{
	char mappath[260];
	char buf[1024];
	sprintf(mappath, "%s", gEngfuncs.pfnGetLevelName());
	int len = strlen(mappath);
	mappath[len-3] = 't';
	mappath[len-2] = 'x';
	mappath[len-1] = 't';
	if(!g_pFileSystem->FileExists(mappath))
	{
		gEngfuncs.pfnConsolePrint(va("Could not found %s.txt. failed to load mapinfo.\n", mappath));
		return;
	}
	FileHandle_t file = g_pFileSystem->Open(mappath, "r");
	char *p = buf;
	g_MapInfo.buf.clear();
	while(p && *p)
	{
		p = g_pFileSystem->ReadLine(buf, 1023, file);
		g_MapInfo.buf.append(buf);
	}
	g_pFileSystem->Close(file);
	MapInfo_ParseBuf();
}

void MapInfo_AddToBuf(const char *str)
{
	g_MapInfo.buf.append(str);
}

void MapInfo_ClearBuf(void)
{
	g_MapInfo.buf.clear();
}

void MapInfo_Init(void)
{
	g_MapInfo.name.clear();
	g_MapInfo.mode.clear();
	g_MapInfo.brief.clear();
	g_MapInfo.image.clear();
	g_MapInfo.buf.clear();
	g_MapInfo.hasinfo = 0;
}

void HudIntroMenu_Close(void)
{
	if(g_iTeam == 0)
	{
		g_iMenu = MENU_CHOOSETEAM;
		gEngfuncs.pfnClientCmd("chooseteam\n");
	}
	else
	{
		g_iMenu = 0;
		HudBase_ActivateMouse();
		gEngfuncs.pfnClientCmd("closemenu\n");
	}
}

int HudIntroMenu_Redraw(float flTime, int iIntermission)
{
	if(g_iMenu != MENU_INTRO)
		return 0;

	int r, g, b;
	int r2, g2, b2;
	int mx, my;

	gEngfuncs.GetMousePosition(&mx, &my);

	g_pSurface->DrawSetColor(255,255,255,255);
	g_pSurface->DrawSetTexture(g_texMenu);
	g_pSurface->DrawTexturedRect(g_xywhMenu.x, g_xywhMenu.y, g_xywhMenu.x+g_xywhMenu.w, g_xywhMenu.y+g_xywhMenu.h);

	if(g_iPage == 1 && g_MapInfo.hasinfo && !g_MapInfo.image.empty())
	{
		std::list<mapimage_t>::iterator it;
		for(it = g_MapInfo.image.begin(); it !=  g_MapInfo.image.end(); it++)
		{
			g_pSurface->DrawSetTexture(it->tex);
			DrawPointsQuad(&(it->p[0]));
		}
	}

	if(g_iPage == 0)
	{
		g_pSurface->DrawSetTextColor(236,236,236,255);
		g_pSurface->DrawSetTextFont(g_hTitleFont);
		g_pSurface->DrawSetTextPos(g_xyTitleFont.x, g_xyTitleFont.y);
		g_pSurface->DrawPrintText(g_Motd.title.c_str(), g_Motd.title.length());

		g_pSurface->DrawSetTextFont(g_hIntroFont);
		HudBase_DrawMultilineSetup(g_lineOffset, 128 * ScreenHeight / 480);
		HudBase_DrawMultiline(g_Motd.text.c_str(), g_xyIntroFont.x, g_xyIntroFont.y);
	}
	else if (g_iPage == 1 && g_MapInfo.hasinfo)
	{
		g_pSurface->DrawSetTextColor(236,236,236,255);
		g_pSurface->DrawSetTextFont(g_hTitleFont);
		g_pSurface->DrawSetTextPos(g_xyTitleFont.x, g_xyTitleFont.y);
		g_pSurface->DrawPrintText(g_MapInfo.name.c_str(), g_MapInfo.name.length());

		int ln = 0;
		int introX = g_xyIntroFont.x;
		int introY = g_xyIntroFont.y;
		g_pSurface->DrawSetTextFont(g_hIntroFont);
		HudBase_DrawMultilineSetup(g_lineOffset, 360 * ScreenHeight / 480);		
		introY += ln * 20 * ScreenHeight / 480;
		ln = HudBase_DrawMultiline(g_MapInfo.mode.c_str(), introX, introY);
		introX += 10 * ScreenHeight / 480;
		introY += ln * 20 * ScreenHeight / 480;
		ln = HudBase_DrawMultiline(g_MapInfo.brief.c_str(), introX, introY);
	}

	r = 118; g = 107; b = 97;
	r2 = 69; g2 = 63; b2 = 57;
	if(HudBase_IsMouseInRect(mx, my, g_xywhNext.x, g_xywhNext.y, g_xywhNext.w, g_xywhNext.h))
	{
		r = 146; g = 71; b = 56;
		r2 = 233; g2 = 222; b2 = 200;
	}
	else
	{
		r = 118; g = 107; b = 97;
		r2 = 69; g2 = 63; b2 = 57;
	}
	g_pSurface->DrawSetColor(r, g, b, 255);
	DrawButton(g_xywhNext.x, g_xywhNext.y, g_xywhNext.w, g_xywhNext.h);
	g_pSurface->DrawSetTextColor(r2, g2, b2, 255);
	DrawButtonLabel(g_wszNext, g_xywhNext.x, g_xywhNext.y, g_xywhNext.w, g_xywhNext.h);

	if(g_iPage == 1)
	{
		if(HudBase_IsMouseInRect(mx, my, g_xywhPrev.x, g_xywhPrev.y, g_xywhPrev.w, g_xywhPrev.h))
		{
			r = 146; g = 71; b = 56;
			r2 = 233; g2 = 222; b2 = 200;
		}
		else
		{
			r = 118; g = 107; b = 97;
			r2 = 69; g2 = 63; b2 = 57;
		}
		g_pSurface->DrawSetColor(r, g, b, 255);
		DrawButton(g_xywhPrev.x, g_xywhPrev.y, g_xywhPrev.w, g_xywhPrev.h);
		g_pSurface->DrawSetTextColor(r2, g2, b2, 255);
		DrawButtonLabel(g_wszPrev, g_xywhPrev.x, g_xywhPrev.y, g_xywhPrev.w, g_xywhPrev.h);
	}

	g_pSurface->DrawFlushText();

	return 1;
}

void HudIntroMenu_MouseUp(int mx, int my)
{
	if(g_iMenu != MENU_INTRO)
		return;

	if(HudBase_IsMouseInRect(mx, my, g_xywhNext.x, g_xywhNext.y, g_xywhNext.w, g_xywhNext.h))
	{
		if(g_iPage == 1)
		{
			HudIntroMenu_Close();
		}
		else if(g_iPage == 0)
		{
			HudIntroMenu_SetPage(1);
		}
		return;
	}
	if(HudBase_IsMouseInRect(mx, my, g_xywhPrev.x, g_xywhPrev.y, g_xywhPrev.w, g_xywhPrev.h) && g_iPage == 1)
	{
		HudIntroMenu_SetPage(0);
		return;
	}
}

int HudIntroMenu_KeyEvent(int eventcode, int keynum, const char *pszCurrentBinding)
{
	if(g_iMenu != MENU_INTRO)
		return 0;

	if(keynum == 'c' || keynum == 'z')
	{
		if(eventcode != 0)
			return 1;
		if(keynum == 'c')
		{
			if(g_iPage == 1)
			{
				HudIntroMenu_Close();
			}
			else if(g_iPage == 0)
			{
				HudIntroMenu_SetPage(1);
			}
		}
		else
		{
			if(g_iPage == 1)
			{
				HudIntroMenu_SetPage(0);
			}
		}
		return 1;
	}
	return 0;
}