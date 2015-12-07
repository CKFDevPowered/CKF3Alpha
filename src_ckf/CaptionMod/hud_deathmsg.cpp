#include "hud_base.h"
#include "exportfuncs.h"
#include "client.h"
#include "cvar.h"
#include "util.h"
#include <list>
#include <vector>

typedef struct
{
	char name[32];
	int texid;
	int x, y, w, h;
}deathicon_t;

typedef struct
{
	char killer[128];
	char victim[64];
	deathicon_t *icon;
	colorVec killercol;
	colorVec victimcol;
	colorVec iconcol;
	colorVec bgcol;
	int w, h;
	int kw, kh;
	int critkill;
}deathpanel_t;

static deathicon_t *g_SuicideIcon;
static deathicon_t *g_CritKillIcon;

static xywh_t g_xywhPanel;
static float g_flLastPanel;
static std::vector<deathicon_t> g_DeathIcons;
static std::list<deathpanel_t> g_DeathPanels;

static void LoadDeathIcon(const char *name, int x, int y)
{
	deathicon_t icon;

	strncpy(icon.name, name, 31);
	icon.x = x;
	icon.y = y;

	char fullpath[256];
	sprintf(fullpath, "resource\\tga\\d_%s", name);

	icon.texid = Surface_LoadTGA(fullpath);
	g_pSurface->DrawGetTextureSize(icon.texid, icon.w, icon.h);

	g_DeathIcons.push_back(icon);
}

static deathicon_t *FindDeathIcon(const char *name)
{
	std::vector<deathicon_t>::iterator it;
	for(it = g_DeathIcons.begin(); it != g_DeathIcons.end(); it++)
	{
		if(!stricmp(it->name, name))
			return &(*it);
	}
	return NULL;
}

int HudDeathMsg_VidInit(void)
{
	return 1;

	g_DeathIcons.clear();
	g_DeathIcons.reserve(50);

	//special icon
	LoadDeathIcon("suicide", 0, 0);
	LoadDeathIcon("critical", 0, 0);
	LoadDeathIcon("backstab", 0, 0);
	LoadDeathIcon("bat", 0, 0);
	LoadDeathIcon("blucap_b", 0, 0);
	LoadDeathIcon("blucap_d", 0, 0);
	LoadDeathIcon("bludef_b", 0, 0);
	LoadDeathIcon("bludef_d", 0, 0);
	LoadDeathIcon("bonesaw", 0, 0);
	LoadDeathIcon("bottle", 0, 0);
	LoadDeathIcon("butterfly", 0, 0);	
	LoadDeathIcon("defgrenade", 0, 0);
	LoadDeathIcon("defrocket", 0, 0);
	LoadDeathIcon("defsticky", 0, 0);
	LoadDeathIcon("dom_b", 0, 0);
	LoadDeathIcon("dom_d", 0, 0);
	LoadDeathIcon("fireaxe", 0, 0);
	LoadDeathIcon("fist", 0, 0);
	LoadDeathIcon("flame_b", 0, 0);
	LoadDeathIcon("flame_d", 0, 0);
	LoadDeathIcon("grenade", 0, 0);
	LoadDeathIcon("headshot", 0, 0);
	LoadDeathIcon("kukri", 0, 0);
	LoadDeathIcon("minigun", 0, 0);
	LoadDeathIcon("pistol", 0, 0);
	LoadDeathIcon("redcap_b", 0, 0);
	LoadDeathIcon("redcap_d", 0, 0);
	LoadDeathIcon("reddef_b", 0, 0);
	LoadDeathIcon("reddef_d", 0, 0);
	LoadDeathIcon("revolver", 0, 0);
	LoadDeathIcon("rocket", 0, 0);
	LoadDeathIcon("sapper_b", 0, 0);
	LoadDeathIcon("sapper_d", 0, 0);
	LoadDeathIcon("scattergun", 0, 0);
	LoadDeathIcon("sentry1", 0, 0);
	LoadDeathIcon("sentry2", 0, 0);
	LoadDeathIcon("sentry3", 0, 0);
	LoadDeathIcon("shotgun", 0, 0);
	LoadDeathIcon("shovel", 0, 0);
	LoadDeathIcon("smg", 0, 0);
	LoadDeathIcon("sniperifle", 0, 0);
	LoadDeathIcon("sticky", 0, 0);
	LoadDeathIcon("syringe", 0, 0);
	LoadDeathIcon("wrench", 0, 0);

	g_SuicideIcon = FindDeathIcon("suicide");
	g_CritKillIcon = FindDeathIcon("critical");

	g_xywhPanel.x = ScreenWidth * 0.9;
	g_xywhPanel.y = ScreenHeight *.05;
	g_xywhPanel.w = 8;
	g_xywhPanel.h = 24;
	return 1;
}

void HudDeathMsg_Init(void)
{
	g_DeathPanels.clear();	
}

int HudDeathMsg_Redraw(float flTime, int iIntermission)
{
	return 0;

	if(iIntermission)
		return 0;

	std::list<deathpanel_t>::iterator it;

	float cleantime = min(max(g_deathmsg_clean->value, 1.0), 30.0);
	if(!g_DeathPanels.empty() && flTime - g_flLastPanel > cleantime)
	{
		g_DeathPanels.pop_front();
		g_flLastPanel = flTime;
	}

	int x, y;
	int x2, y2;

	x = g_xywhPanel.x;
	y = g_xywhPanel.y;
	for(it = g_DeathPanels.begin(); it != g_DeathPanels.end(); it++)
	{
		g_pSurface->DrawSetColor(it->bgcol.r, it->bgcol.g, it->bgcol.b, 255);

		x2 = x - it->w;
		DrawButtonEx(x2, y, it->w, it->h, 6);

		x2 += g_xywhPanel.w;
		y2 = y + (it->h - it->kh) / 2;
		gEngfuncs.pfnDrawSetTextColor(it->killercol.r / 255.0, it->killercol.g / 255.0, it->killercol.b / 255.0);
		gEngfuncs.pfnDrawConsoleString(x2, y2, it->killer);
		x2 += it->kw + it->icon->x;

		g_pSurface->DrawSetColor(it->iconcol.r, it->iconcol.g, it->iconcol.b, 255);
		g_pSurface->DrawSetTexture(it->icon->texid);
		g_pSurface->DrawTexturedRect(x2, y+it->icon->y, x2+it->icon->w * 0.9, y+it->icon->y+it->icon->h * 0.9);

		if(it->critkill)
		{
			g_pSurface->DrawSetColor(255, 255, 255, 255);
			g_pSurface->DrawSetTexture(g_CritKillIcon->texid);
			g_pSurface->DrawTexturedRect(x2, y+it->icon->y, x2+it->icon->w * 0.9, y+it->icon->y+it->icon->h * 0.9);
		}

		x2 += it->icon->w * 0.9 + g_xywhPanel.w;
		gEngfuncs.pfnDrawSetTextColor(it->victimcol.r / 255.0, it->victimcol.g / 255.0, it->victimcol.b / 255.0);
		gEngfuncs.pfnDrawConsoleString(x2, y2, it->victim);

		y = y + it->h + 8;
	}
	return 1;
}

static void FillTeamColor(colorVec *col, int team)
{
	col->a = 255;
	if(team == 1)
	{
		col->r = 214;
		col->g = 88;
		col->b = 76;
	}
	else if(team == 2)
	{
		col->r = 81;
		col->g = 137;
		col->b = 174;
	}
	else
	{
		col->r = 255;
		col->g = 255;
		col->b = 255;
	}
}

void HudDeathMsg_AddPanel(const char *killer, const char *assister, int killerteam, const char *victim, int victimteam, const char *iconname, int relative, int critkill)
{
	/*deathicon_t *icon;
	char iconname2[32];
	bool iconcolored;
	
	icon = FindDeathIcon(iconname);
	iconcolored = false;

	if(!icon)
	{		
		sprintf(iconname2, relative ? "%s_d" : "%s_d", iconname);
		icon = FindDeathIcon(iconname2);
		if(icon) iconcolored = true;
	}
	if(!icon)
		icon = FindDeathIcon("suicide");
	if(!icon)
		return;

	deathpanel_t panel;

	if(!killer)
		panel.killer[0] = 0;
	else if(!assister)
		strcpy(panel.killer, killer);
	else
		sprintf(panel.killer, "%s + %s", killer, assister);

	strcpy(panel.victim, victim);

	FillTeamColor(&panel.killercol, killerteam);
	FillTeamColor(&panel.victimcol, victimteam);

	panel.icon = icon;
	panel.iconcol.a = 255;
	panel.bgcol.a = 255;
	panel.critkill = critkill;
	if(relative)
	{
		if(!iconcolored)
		{
			panel.iconcol.r = 63;
			panel.iconcol.g = 57;
			panel.iconcol.b = 35;
		}
		panel.bgcol.r = 220;
		panel.bgcol.g = 206;
		panel.bgcol.b = 177;
	}
	else
	{
		if(!iconcolored)
		{
			panel.iconcol.r = 222;
			panel.iconcol.g = 213;
			panel.iconcol.b = 175;
		}
		panel.bgcol.r = 39;
		panel.bgcol.g = 36;
		panel.bgcol.b = 35;
	}
	if(iconcolored)
	{
		panel.iconcol.r = 255;
		panel.iconcol.g = 255;
		panel.iconcol.b = 255;
	}

	int w, w2, h;

	gEngfuncs.pfnDrawConsoleStringLen(panel.killer, &w, &h);
	panel.kw = w;
	panel.kh = h;

	gEngfuncs.pfnDrawConsoleStringLen(panel.victim, &w2, &h);

	panel.w = w + icon->x + icon->w * 0.9 + w2 + g_xywhPanel.w * 4;
	panel.h = max(g_xywhPanel.h, icon->h * 0.9);

	g_DeathPanels.push_back(panel);

	int maxnum = min(max((int)g_deathmsg_max->value, 1), 10);
	if(!g_DeathPanels.empty() && g_DeathPanels.size() > maxnum)
		g_DeathPanels.pop_front();

	g_flLastPanel = gEngfuncs.GetClientTime();*/
}