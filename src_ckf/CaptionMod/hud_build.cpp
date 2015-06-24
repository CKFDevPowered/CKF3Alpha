#include "hud_base.h"
#include "client.h"
#include "cvar.h"
#include "qgl.h"
#include "util.h"

#define DISABLED 0
#define	RED 1
#define	BLU 2

#define ALERT_SAPPED 2
#define ALERT_DAMAGED 1
#define ALERT_NOFLASH_DAMAGED 0

#define ALERTSTATE_IDLE 0
#define ALERTSTATE_SHOWING 1
#define ALERTSTATE_DISPLAY 2
#define ALERTSTATE_HIDING 3

#define ALERT_DURATION (0.8)

#define HEALTH_CHANGERATE 4
#define BUILDSPEED_SENTRY 10
#define BUILDSPEED_DISPENSER 20
#define BUILDSPEED_TELEPORT 20

static color24 g_ColorRed ={255,0,0};
static color24 g_ColorWhite = {255,255,255};
static color24 g_ColorGray = {140,140,140};
static color24 g_ColorNorm = {251,236,203};

static flcol24_t g_flColor[] = {{236/255.0f,227/255.0f,203/255.0f}};

void DrawSentry(void);
void DrawDispenser(void);
void DrawTelein(void);
void DrawTeleout(void);
void DrawLabel(int x,int y, const wchar_t *wszstr);

inline void SetColor(color24 c)
{
	g_pSurface->DrawSetColor(c.r, c.g, c.b, 255);
}

inline int Screen640(int c)
{
	return c*ScreenWidth/640;
}

class cMask
{
public:
	int m_tex[3];
	xywh_t m_xywh;
	cMask()
	{
		m_tex[0] = 0;
		m_tex[1] = 0;
		m_tex[2] = 0;
		m_xywh.x = 0;
		m_xywh.y = 0;
		m_xywh.w = 0;
		m_xywh.h = 0;
	}
	void Copy(cMask *other)
	{
		m_tex[0] = other->m_tex[0];
		m_tex[1] = other->m_tex[1];
		m_tex[2] = other->m_tex[2];
		m_xywh = other->m_xywh;
	}

	void Draw(int type)
	{
		SetColor(g_ColorNorm);
		g_pSurface->DrawSetTexture(m_tex[type]);
		g_pSurface->DrawTexturedRect(m_xywh.x,m_xywh.y,m_xywh.w+m_xywh.x,m_xywh.h+m_xywh.y);
	}

	void DrawWhite(int type)
	{
		SetColor(g_ColorWhite);
		g_pSurface->DrawSetTexture(m_tex[type]);
		g_pSurface->DrawTexturedRect(m_xywh.x,m_xywh.y,m_xywh.w+m_xywh.x,m_xywh.h+m_xywh.y);
	}

	void Draw(void)
	{
		Draw(0);
	}

	void DrawWhite(void)
	{
		DrawWhite(0);
	}
};

static cMask g_SentryMask;
static cMask g_SentryAlertMask;
static cMask g_SentryIcon;
static cMask g_SentryLevelNumberIcon;
static cMask g_SentryKillIcon;
static cMask g_SentryAmmoIcon;
static cMask g_SentryUpgradeIcon;
static cMask g_SentryRocketIcon;
static xy_t g_xySentryDisabledText;
static xy_t g_xySentryKillCountText;
static xy_t g_xySentryBuildingText;

static cMask g_DispenserIcon;
static cMask g_DispenserLevelNumberIcon;
static cMask g_DispenserAmmoIcon;
static cMask g_DispenserUpgradeIcon;
static xy_t g_xyDispenserText;
static xywh_t g_DispenserAmmoBar;
static xywh_t g_DispenserUpgradeBar;
static xywh_t g_DispenserBuildingBar;

static cMask g_TeleinIcon;
static cMask g_TeleinLevelNumberIcon;
static cMask g_TeleinCountIcon;
static cMask g_TeleinUpgradeIcon;
static xywh_t g_TeleinUpgradeBar;
static xywh_t g_TeleinBuildingBar;
static xy_t g_xyTeleinText;
static xy_t g_xyTeleinCountText;

static cMask g_TeleoutIcon;
static cMask g_TeleoutLevelNumberIcon;
static cMask g_TeleoutUpgradeIcon;
static xywh_t g_TeleoutUpgradeBar;
static cMask g_TeleoutProgressIcon;
static xywh_t g_TeleoutProgressBar;
static xywh_t g_TeleoutBuildingBar;
static xy_t g_xyTeleoutText;

static cMask g_CommonMask[3];
static cMask g_CommonAlertMask[3];

static cMask g_AlertWrenchIcon[4];
static cMask g_AlertSapperIcon[4];

typedef struct
{
	int state;
	int type;
	float start;
}buildalert_t;

static buildalert_t g_Alert[4];

static xywh_t g_HealthBar[4];
static int g_HealthBarBreaklen[2];
static xywh_t g_SentryAmmoBar;
static xywh_t g_SentryUpgradeBar;
static xywh_t g_SentryBuildingBar;

static wchar_t wszTeleportCount[4];

static void DrawBreakFillBar(xywh_t *t,float percent,color24 *_col)
{
	g_pSurface->DrawSetColor(_col->r, _col->g, _col->b, 255);
	int iBreakYpos = 0;
	int iHeight = percent*t->h;
	while(1)//up = y-h
	{
		if(iBreakYpos+g_HealthBarBreaklen[0]+g_HealthBarBreaklen[1] < iHeight)
		{
			g_pSurface->DrawFilledRect(t->x,t->y-iBreakYpos-g_HealthBarBreaklen[0],t->w+t->x,t->y-iBreakYpos);
		}
		else
		{
			//g_pSurface->DrawFilledRect(t->x,t->y-iHeight,t->w+t->x,t->y-iBreakYpos);
			g_pSurface->DrawFilledRect(t->x,t->y-iBreakYpos-g_HealthBarBreaklen[0],t->w+t->x,t->y-iBreakYpos);
			break;
		}
		iBreakYpos += (g_HealthBarBreaklen[0]+g_HealthBarBreaklen[1]);
	}
}

static void DrawLabel(int x, int y, const wchar_t *wszstr)
{
	gEngfuncs.pfnDrawSetTextColor(g_flColor[0].r, g_flColor[0].g, g_flColor[0].b);
	gEngfuncs.pfnDrawConsoleString(x, y, UnicodeToUTF8(wszstr));
}

static void SetAlert(int i, bool show)
{
	buildalert_t *alert = &g_Alert[i];
	float time = gEngfuncs.GetClientTime();
	if(show)
	{
		if(alert->state == ALERTSTATE_IDLE)
		{
			alert->start = time;
			alert->state = ALERTSTATE_SHOWING;
		}
		else if(alert->state == ALERTSTATE_HIDING)
		{
			float frac = (time - alert->start) / ALERT_DURATION;
			alert->start = time - frac * ALERT_DURATION;
			alert->state = ALERTSTATE_SHOWING;
		}
	}
	else
	{
		if(alert->state == ALERTSTATE_DISPLAY)
		{
			alert->start = time;
			alert->state = ALERTSTATE_HIDING;
		}
		else if(alert->state == ALERTSTATE_SHOWING)
		{
			float frac = (time - alert->start) / ALERT_DURATION;
			alert->start = time - frac * ALERT_DURATION;
			alert->state = ALERTSTATE_HIDING;
		}
	}
}

static void DrawAlert(int i)
{
	int x,w,y,h,iPanelStart,iPanelLength,iPanelEnd;
	float flAlert;
	cMask *m;

	buildalert_t *alert = &g_Alert[i];

	if(alert->state == ALERTSTATE_IDLE)
		return;

	if(alert->state == ALERTSTATE_SHOWING)
	{
		flAlert = (gEngfuncs.GetClientTime() - alert->start) / ALERT_DURATION;
		if(flAlert >= 1)
		{
			alert->state = ALERTSTATE_DISPLAY;
			flAlert = 1;
		}
	}
	else if(alert->state == ALERTSTATE_HIDING)
	{
		flAlert = (gEngfuncs.GetClientTime() - alert->start) / ALERT_DURATION;
		if(flAlert >= 1)
		{
			alert->state = ALERTSTATE_IDLE;
			return;
		}
		flAlert = 1-flAlert;
	}
	else
	{
		flAlert = 1;
	}

	if(i == 0)
	{
		m = &g_SentryAlertMask;
		iPanelLength = m->m_xywh.w;
		iPanelEnd = 11;
		iPanelStart = 6;
	}
	else
	{
		m = &g_CommonAlertMask[i-1];
		iPanelLength = m->m_xywh.w;
		iPanelEnd = 7;
		iPanelStart = 4;
	}

	x=m->m_xywh.x;
	w=m->m_xywh.w;
	y=m->m_xywh.y;
	h=m->m_xywh.h;

	qglEnable(GL_TEXTURE_2D);
	qglEnable(GL_BLEND);
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	qglBindTexture(GL_TEXTURE_2D, m->m_tex[0]);

	if(alert->type == ALERT_NOFLASH_DAMAGED)
		qglColor4ub(g_ColorWhite.r,g_ColorWhite.g,g_ColorWhite.b,255);
	else
		qglColor4ub(g_ColorRed.r,g_ColorRed.g,g_ColorRed.b,255);

	//draw left

	qglBegin(GL_QUADS);
	qglTexCoord2f(0,0);//x0y0
	qglVertex3f(x,y,0);
	qglTexCoord2f(0.0625f,0);//x0y1
	qglVertex3f(x+Screen640(iPanelStart),y,0);
	qglTexCoord2f(0.0625f,1);//x1y0
	qglVertex3f(x+Screen640(iPanelStart),y+h,0);	
	qglTexCoord2f(0,1);//x1y1
	qglVertex3f(x,y+h,0);
	qglEnd();

	x += Screen640(iPanelStart);

	qglBegin(GL_QUADS);
	qglTexCoord2f(0.484375-0.421875*flAlert,0);//x0y0
	qglVertex3f(x,y,0);
	qglTexCoord2f(0.5703125,0);//x1y0
	qglVertex3f(x+Screen640(iPanelEnd+iPanelLength*flAlert),y,0);
	qglTexCoord2f(0.5703125,1);//x1y1
	qglVertex3f(x+Screen640(iPanelEnd+iPanelLength*flAlert),y+h,0);	
	qglTexCoord2f(0.484375-0.421875*flAlert,1);//x0y1
	qglVertex3f(x,y+h,0);
	qglEnd();

	if(alert->state == ALERTSTATE_DISPLAY)
	{
		cMask *mIcon;
		if(alert->type == ALERT_SAPPED)
			mIcon = &g_AlertSapperIcon[i];
		else
			mIcon = &g_AlertWrenchIcon[i];
		mIcon->DrawWhite();
	}
}

void DrawHealthBar(int i, float percent)
{
	if(percent > 0.4)
		DrawBreakFillBar(&g_HealthBar[i], percent, &g_ColorNorm);
	else
		DrawBreakFillBar(&g_HealthBar[i], percent, &g_ColorRed);
}

static buildable_t *GetBuildable(int i)
{
	buildable_t *build;
	switch(i)
	{
		case 0:build = (buildable_t *)(&g_Build.sentry);break;
		case 1:build = (buildable_t *)(&g_Build.dispenser);break;
		case 2:build = (buildable_t *)(&g_Build.telein);break;
		case 3:build = (buildable_t *)(&g_Build.teleout);break;
	}
	return build;
}

static float GetUpdateProgress(int i, int upgrade)
{
	switch(i)
	{
	case 0:
		return (upgrade) ? 5.0 : 1.0;
	case 1:
		return (upgrade) ? 5.0 : 0.5;
	case 2:
		return (upgrade) ? 100.0 : 0.5;
	case 3:
		return (upgrade) ? 100.0 : 0.5;
	}
	return 0;
}

void UpdateTeleport(void)
{
	if(g_Build.telein.level == 0)
		return;
	if(g_Build.telein.flags != 0)
		return;
	if(g_Build.telein.chargerate == 0)
		return;
	if(g_Build.telein.charge >= 100.0)
		return;

	if(gEngfuncs.GetClientTime() - g_Build.telein.chargetime > 0.1)
	{
		g_Build.telein.charge += g_Build.telein.chargerate * 0.1;
		g_Build.telein.chargetime = gEngfuncs.GetClientTime();
		if(g_Build.telein.charge > 100.0)
			g_Build.telein.charge = 100.0;
	}
}

void UpdateBuild(int i)
{
	buildable_t *build = GetBuildable(i);

	if(build->level == 0)
		return;

	if(!(build->flags & BUILD_BUILDING) && !(build->flags & BUILD_UPGRADING))
		return;

	float flNextMaxHealth;
	float flCurMaxHealth;
	float flProgressIncrease;

	if(gEngfuncs.GetClientTime() - build->updatetime > 0.1)
	{
		if(build->flags & BUILD_BUILDING)
		{
			flNextMaxHealth = 150;
			flCurMaxHealth = 1;
			flProgressIncrease = GetUpdateProgress(i, 0);
		}
		else
		{
			if(build->level == 2)
			{
				flNextMaxHealth = 180;
				flCurMaxHealth = 150;
			}
			else if(build->level == 3)
			{
				flNextMaxHealth = 216;
				flCurMaxHealth = 180;
			}
			flProgressIncrease = GetUpdateProgress(i, 1);;
		}
		if(build->progress + flProgressIncrease > 100.0)
			flProgressIncrease = 100.0 - build->progress;

		build->health = min(build->health + flProgressIncrease * (flNextMaxHealth-flCurMaxHealth)/100.0, flNextMaxHealth);

		if(build->health > build->maxhealth)
			build->maxhealth = build->health;

		build->progress += flProgressIncrease;
		build->updatetime = gEngfuncs.GetClientTime();
	}
}

void UpdateBuildables(void)
{
	if(g_iClass != CLASS_ENGINEER)
		return;

	UpdateBuild(0);
	UpdateBuild(1);
	UpdateBuild(2);
	UpdateBuild(3);

	
	{
		UpdateTeleport();
	}
}

#define BUILD_X 6
#define BUILD_Y 5

int HudBuild_VidInit(void)
{
	g_SentryMask.m_tex[0] = Surface_LoadTGA("resource\\tga\\hud_sentry_disabled");
	g_SentryMask.m_tex[1] = Surface_LoadTGA("resource\\tga\\hud_sentry_red");
	g_SentryMask.m_tex[2] = Surface_LoadTGA("resource\\tga\\hud_sentry_blu");
	g_SentryMask.m_xywh.x = Screen640(BUILD_X);
	g_SentryMask.m_xywh.y = Screen640(BUILD_Y);
	g_SentryMask.m_xywh.w = Screen640(108);
	g_SentryMask.m_xywh.h = Screen640(59);
	g_SentryAlertMask.m_tex[0] = Surface_LoadTGA("resource\\tga\\hud_sentry_alert");//,133,BUILD_Y,54,54);
	g_SentryAlertMask.m_xywh.x = Screen640(95)+g_SentryMask.m_xywh.x;
	g_SentryAlertMask.m_xywh.y = g_SentryMask.m_xywh.y;
	g_SentryAlertMask.m_xywh.w = Screen640(10);
	g_SentryAlertMask.m_xywh.h = Screen640(59);
	g_SentryIcon.m_tex[0] = Surface_LoadTGA("resource\\tga\\ico_sentry1");
	g_SentryIcon.m_tex[1] = Surface_LoadTGA("resource\\tga\\ico_sentry2");
	g_SentryIcon.m_tex[2] = Surface_LoadTGA("resource\\tga\\ico_sentry3");
	g_SentryIcon.m_xywh.x = Screen640(20)+g_SentryMask.m_xywh.x;
	g_SentryIcon.m_xywh.y = Screen640(9)+g_SentryMask.m_xywh.y;
	g_SentryIcon.m_xywh.w = g_SentryIcon.m_xywh.h = Screen640(30);
	g_SentryLevelNumberIcon.m_tex[0] = Surface_LoadTGA("resource\\tga\\hud_build_level1");
	g_SentryLevelNumberIcon.m_tex[1] = Surface_LoadTGA("resource\\tga\\hud_build_level2");
	g_SentryLevelNumberIcon.m_tex[2] = Surface_LoadTGA("resource\\tga\\hud_build_level3");
	g_SentryLevelNumberIcon.m_xywh.x = Screen640(40)+g_SentryMask.m_xywh.x;
	g_SentryLevelNumberIcon.m_xywh.y = Screen640(6)+g_SentryMask.m_xywh.y;
	g_SentryLevelNumberIcon.m_xywh.w = g_SentryLevelNumberIcon.m_xywh.h = Screen640(8);
	g_SentryKillIcon.m_tex[0] = Surface_LoadTGA("resource\\tga\\ico_kill");
	g_SentryKillIcon.m_xywh.x = Screen640(52)+g_SentryMask.m_xywh.x;
	g_SentryKillIcon.m_xywh.y = Screen640(10)+g_SentryMask.m_xywh.y;
	g_SentryKillIcon.m_xywh.w = g_SentryKillIcon.m_xywh.h = Screen640(8);
	g_SentryAmmoIcon.m_tex[0] = Surface_LoadTGA("resource\\tga\\ico_ammo");
	g_SentryAmmoIcon.m_xywh.x = g_SentryKillIcon.m_xywh.x;
	g_SentryAmmoIcon.m_xywh.y = Screen640(20)+g_SentryMask.m_xywh.y;
	g_SentryAmmoIcon.m_xywh.w = g_SentryAmmoIcon.m_xywh.h = Screen640(8);
	g_SentryUpgradeIcon.m_tex[0] = Surface_LoadTGA("resource\\tga\\ico_metal");
	g_SentryUpgradeIcon.m_xywh.x = g_SentryKillIcon.m_xywh.x;
	g_SentryUpgradeIcon.m_xywh.y = Screen640(30)+g_SentryMask.m_xywh.y;
	g_SentryUpgradeIcon.m_xywh.w = g_SentryUpgradeIcon.m_xywh.h = Screen640(8);
	g_SentryRocketIcon.m_tex[0] = Surface_LoadTGA("resource\\tga\\ico_rocket");
	g_SentryRocketIcon.m_xywh.x = g_SentryKillIcon.m_xywh.x;
	g_SentryRocketIcon.m_xywh.y = Screen640(30)+g_SentryMask.m_xywh.y;
	g_SentryRocketIcon.m_xywh.w = g_SentryRocketIcon.m_xywh.h = Screen640(8);
	g_SentryAmmoBar.x = Screen640(62)+g_SentryMask.m_xywh.x;
	g_SentryAmmoBar.y = Screen640(21)+g_SentryMask.m_xywh.y;
	g_SentryAmmoBar.w = Screen640(32);
	g_SentryAmmoBar.h = Screen640(7);
	g_SentryUpgradeBar = g_SentryAmmoBar;
	g_SentryUpgradeBar.y = Screen640(31)+g_SentryMask.m_xywh.y;
	g_xySentryKillCountText.x = Screen640(63)+g_SentryMask.m_xywh.x;
	g_xySentryKillCountText.y = Screen640(11)+g_SentryMask.m_xywh.y;
	g_xySentryBuildingText.x = Screen640(50)+g_SentryMask.m_xywh.x;
	g_xySentryBuildingText.y = Screen640(15)+g_SentryMask.m_xywh.y;
	g_xySentryDisabledText.x = Screen640(50)+g_SentryMask.m_xywh.x;
	g_xySentryDisabledText.y = Screen640(17)+g_SentryMask.m_xywh.y;
	g_SentryBuildingBar.x = Screen640(50)+g_SentryMask.m_xywh.x;
	g_SentryBuildingBar.y = Screen640(25)+g_SentryMask.m_xywh.y;
	g_SentryBuildingBar.w = Screen640(42);
	g_SentryBuildingBar.h = Screen640(6);

	g_CommonMask[0].m_tex[0] = Surface_LoadTGA("resource\\tga\\hud_tele_disabled");
	g_CommonMask[0].m_tex[1] = Surface_LoadTGA("resource\\tga\\hud_tele_red");
	g_CommonMask[0].m_tex[2] = Surface_LoadTGA("resource\\tga\\hud_tele_blu");
	g_CommonMask[0].m_xywh.x = g_SentryMask.m_xywh.x;
	g_CommonMask[0].m_xywh.w = g_SentryMask.m_xywh.w;
	g_CommonMask[0].m_xywh.h = Screen640(30);
	g_CommonMask[0].m_xywh.y = g_SentryMask.m_xywh.y+g_SentryMask.m_xywh.h-Screen640(12);
	g_CommonMask[1].Copy(&g_CommonMask[0]);
	g_CommonMask[1].m_xywh.y = g_CommonMask[0].m_xywh.y+g_CommonMask[0].m_xywh.h - Screen640(2);
	g_CommonMask[2].Copy(&g_CommonMask[1]);
	g_CommonMask[2].m_xywh.y = g_CommonMask[1].m_xywh.y+g_CommonMask[1].m_xywh.h - Screen640(2);

	g_CommonAlertMask[0].m_tex[0] = Surface_LoadTGA("resource\\tga\\hud_tele_alert");
	g_CommonAlertMask[0].m_xywh.x = g_SentryAlertMask.m_xywh.x;
	g_CommonAlertMask[0].m_xywh.y = g_CommonMask[0].m_xywh.y;
	g_CommonAlertMask[0].m_xywh.w = Screen640(7);
	g_CommonAlertMask[0].m_xywh.h = g_CommonMask[0].m_xywh.h;	
	g_CommonAlertMask[1].Copy(&g_CommonAlertMask[0]);
	g_CommonAlertMask[1].m_xywh.y = g_CommonMask[1].m_xywh.y;
	g_CommonAlertMask[2].Copy(&g_CommonAlertMask[0]);
	g_CommonAlertMask[2].m_xywh.y = g_CommonMask[2].m_xywh.y;

	g_xyDispenserText.x = g_xySentryDisabledText.x;
	g_xyDispenserText.y = g_CommonMask[0].m_xywh.y + Screen640(7);
	g_xyTeleinText.x = g_xySentryDisabledText.x;
	g_xyTeleinText.y = g_CommonMask[1].m_xywh.y + Screen640(7);
	g_xyTeleoutText.x = g_xySentryDisabledText.x;
	g_xyTeleoutText.y = g_CommonMask[2].m_xywh.y + Screen640(7);

	g_DispenserIcon.m_tex[0] = Surface_LoadTGA("resource\\tga\\ico_dispenser");//,BUILD_X+30,2,32,32);
	g_DispenserIcon.m_xywh.x = g_SentryIcon.m_xywh.x;
	g_DispenserIcon.m_xywh.y = g_CommonMask[0].m_xywh.y + Screen640(2);
	g_DispenserIcon.m_xywh.w = g_DispenserIcon.m_xywh.h = Screen640(25);
	g_TeleinIcon.m_tex[0] = Surface_LoadTGA("resource\\tga\\ico_telein");//,BUILD_X+30,2,32,32);
	g_TeleinIcon.m_xywh.x = g_SentryIcon.m_xywh.x;
	g_TeleinIcon.m_xywh.y = g_CommonMask[1].m_xywh.y + Screen640(2);
	g_TeleinIcon.m_xywh.w = g_TeleinIcon.m_xywh.h = Screen640(25);
	g_TeleoutIcon.m_tex[0] = Surface_LoadTGA("resource\\tga\\ico_teleout");//,BUILD_X+30,2,32,32);
	g_TeleoutIcon.m_xywh.x = g_SentryIcon.m_xywh.x;
	g_TeleoutIcon.m_xywh.y = g_CommonMask[2].m_xywh.y + Screen640(2);
	g_TeleoutIcon.m_xywh.w = g_TeleoutIcon.m_xywh.h = Screen640(25);

	g_DispenserLevelNumberIcon.Copy(&g_SentryLevelNumberIcon);
	g_DispenserLevelNumberIcon.m_xywh.y =g_CommonMask[0].m_xywh.y + Screen640(4);
	g_TeleinLevelNumberIcon.Copy(&g_SentryLevelNumberIcon);
	g_TeleinLevelNumberIcon.m_xywh.y =g_CommonMask[1].m_xywh.y + Screen640(4);
	g_TeleoutLevelNumberIcon.Copy(&g_SentryLevelNumberIcon);
	g_TeleoutLevelNumberIcon.m_xywh.y =g_CommonMask[2].m_xywh.y + Screen640(4);

	g_DispenserAmmoIcon = g_SentryAmmoIcon;
	g_DispenserAmmoIcon.m_xywh.y = g_CommonMask[0].m_xywh.y + Screen640(6);
	g_DispenserAmmoBar = g_SentryAmmoBar;
	g_DispenserAmmoBar.y = g_DispenserAmmoIcon.m_xywh.y + Screen640(1);
	g_DispenserUpgradeIcon = g_SentryUpgradeIcon;
	g_DispenserUpgradeIcon.m_xywh.y = g_CommonMask[0].m_xywh.y + Screen640(16);
	g_DispenserUpgradeBar = g_SentryAmmoBar;
	g_DispenserUpgradeBar.y = g_DispenserUpgradeIcon.m_xywh.y + Screen640(1);
	g_DispenserBuildingBar = g_SentryBuildingBar;
	g_DispenserBuildingBar.y = g_CommonMask[0].m_xywh.y + Screen640(16);

	g_TeleinCountIcon.m_tex[0] = Surface_LoadTGA("resource\\tga\\ico_teleport");
	g_TeleinCountIcon.m_xywh = g_SentryAmmoIcon.m_xywh;
	g_TeleinCountIcon.m_xywh.y = g_CommonMask[1].m_xywh.y + Screen640(6);
	g_xyTeleinCountText.x = g_CommonMask[1].m_xywh.x + Screen640(63);
	g_xyTeleinCountText.y = g_CommonMask[1].m_xywh.y + Screen640(6);
	g_TeleinUpgradeIcon = g_SentryUpgradeIcon;
	g_TeleinUpgradeIcon.m_xywh.y = g_CommonMask[1].m_xywh.y + Screen640(16);
	g_TeleinUpgradeBar = g_SentryAmmoBar;
	g_TeleinUpgradeBar.y = g_TeleinUpgradeIcon.m_xywh.y + Screen640(1);
	g_TeleinBuildingBar = g_SentryBuildingBar;
	g_TeleinBuildingBar.y = g_CommonMask[1].m_xywh.y + Screen640(16);

	g_TeleoutUpgradeIcon = g_SentryUpgradeIcon;
	g_TeleoutUpgradeIcon.m_xywh.y = g_CommonMask[2].m_xywh.y + Screen640(6);
	g_TeleoutUpgradeBar = g_SentryAmmoBar;
	g_TeleoutUpgradeBar.y = g_TeleoutUpgradeIcon.m_xywh.y + Screen640(1);
	g_TeleoutProgressIcon = g_TeleinCountIcon;
	g_TeleoutProgressIcon.m_xywh.y = g_CommonMask[2].m_xywh.y + Screen640(16);
	g_TeleoutProgressBar = g_SentryAmmoBar;
	g_TeleoutProgressBar.y = g_TeleoutProgressIcon.m_xywh.y + Screen640(1);
	g_TeleoutBuildingBar = g_SentryBuildingBar;
	g_TeleoutBuildingBar.y = g_CommonMask[2].m_xywh.y + Screen640(16);

	g_AlertWrenchIcon[0].m_tex[0] = Surface_LoadTGA("resource\\tga\\hud_build_alert_wrench");
	g_AlertWrenchIcon[0].m_xywh.x = g_SentryAlertMask.m_xywh.x + Screen640(10);
	g_AlertWrenchIcon[0].m_xywh.y = g_SentryAlertMask.m_xywh.y + Screen640(12);
	g_AlertWrenchIcon[0].m_xywh.w = g_AlertWrenchIcon[0].m_xywh.h = Screen640(25);
	g_AlertWrenchIcon[1] = g_AlertWrenchIcon[0];
	g_AlertWrenchIcon[1].m_xywh.x = g_CommonAlertMask[0].m_xywh.x + Screen640(6);
	g_AlertWrenchIcon[1].m_xywh.y = g_CommonAlertMask[0].m_xywh.y + Screen640(7);
	g_AlertWrenchIcon[1].m_xywh.w = g_AlertWrenchIcon[1].m_xywh.h = Screen640(15);	
	g_AlertWrenchIcon[2] = g_AlertWrenchIcon[1];
	g_AlertWrenchIcon[2].m_xywh.y = g_CommonAlertMask[1].m_xywh.y + Screen640(7);
	g_AlertWrenchIcon[3] = g_AlertWrenchIcon[1];
	g_AlertWrenchIcon[3].m_xywh.y = g_CommonAlertMask[2].m_xywh.y + Screen640(7);

	g_AlertSapperIcon[0].m_tex[0] = Surface_LoadTGA("resource\\tga\\ico_sapper");
	g_AlertSapperIcon[1].m_tex[0] = g_AlertSapperIcon[0].m_tex[0];
	g_AlertSapperIcon[2].m_tex[0] = g_AlertSapperIcon[0].m_tex[0];
	g_AlertSapperIcon[3].m_tex[0] = g_AlertSapperIcon[0].m_tex[0];
	g_AlertSapperIcon[0].m_xywh = g_AlertWrenchIcon[0].m_xywh;
	g_AlertSapperIcon[1].m_xywh = g_AlertWrenchIcon[1].m_xywh;
	g_AlertSapperIcon[2].m_xywh = g_AlertWrenchIcon[2].m_xywh;
	g_AlertSapperIcon[3].m_xywh = g_AlertWrenchIcon[3].m_xywh;

	g_HealthBar[0].x = Screen640(11)+g_SentryMask.m_xywh.x;
	g_HealthBar[0].y = Screen640(46)+g_SentryMask.m_xywh.y;
	g_HealthBar[0].w = Screen640(7);
	g_HealthBar[0].h = Screen640(42);
	g_HealthBar[1].x = g_HealthBar[0].x;
	g_HealthBar[1].y = Screen640(26)+g_CommonMask[0].m_xywh.y;
	g_HealthBar[1].w = Screen640(7);
	g_HealthBar[1].h = Screen640(23);
	g_HealthBar[2].x = g_HealthBar[0].x;
	g_HealthBar[2].y = Screen640(26)+g_CommonMask[1].m_xywh.y;
	g_HealthBar[2].w = Screen640(7);
	g_HealthBar[2].h = Screen640(23);
	g_HealthBar[3].x = g_HealthBar[0].x;
	g_HealthBar[3].y = Screen640(26)+g_CommonMask[2].m_xywh.y;
	g_HealthBar[3].w = Screen640(7);
	g_HealthBar[3].h = Screen640(23);

	g_HealthBarBreaklen[0] = Screen640(2);
	g_HealthBarBreaklen[1] = Screen640(1);

	return 1;
}

void HudBuild_Init(void)
{
	g_Alert[0].state = ALERTSTATE_IDLE;
	g_Alert[1].state = ALERTSTATE_IDLE;
	g_Alert[2].state = ALERTSTATE_IDLE;
	g_Alert[3].state = ALERTSTATE_IDLE;
}

int HudBuild_Redraw(float flTime, int iIntermission)
{
	if(iIntermission)
		return 0;

	if(!CL_IsAlive() && !g_iUser1)
		return 0;

	if(g_iClass != CLASS_ENGINEER)
		return 0;

	g_ColorRed.g=255*(sin(flTime*HEALTH_CHANGERATE)+1)/2;
	g_ColorRed.b=g_ColorRed.g;

	DrawSentry();
	DrawDispenser();
	DrawTelein();
	DrawTeleout();

	return 1;
}

void DrawBar(xywh_t *t,float fpercent)
{
	g_pSurface->DrawSetColor(g_ColorGray.r, g_ColorGray.g, g_ColorGray.b, 150);
	g_pSurface->DrawFilledRect(t->x,t->y,t->x +t->w,t->y+t->h);
	SetColor(g_ColorNorm);
	g_pSurface->DrawFilledRect(t->x,t->y,t->x +t->w*fpercent,t->y+t->h);
}

static void DrawCommon(int i)
{
	buildable_t *build = GetBuildable(i);

	float percent = build->health/build->maxhealth;
	DrawHealthBar(i, percent);

	if(build->flags & BUILD_SAPPERED)
	{
		g_Alert[i].type = ALERT_SAPPED;
		SetAlert(i, true);
	}
	else if((build->flags & BUILD_BUILDING) || ((build->flags & BUILD_UPGRADING) && percent > 0.8) || percent > 0.99)
	{
		SetAlert(i, false);
	}
	else if(percent > 0.4)
	{
		g_Alert[i].type = ALERT_NOFLASH_DAMAGED;
		SetAlert(i, true);
	}
	else
	{
		g_Alert[i].type = ALERT_DAMAGED;
		SetAlert(i, true);
	}
	DrawAlert(i);
}

void DrawTeleout(void)
{
	if(g_Build.teleout.level == 0)
	{
		g_CommonMask[2].DrawWhite(DISABLED);
		DrawLabel(g_xyTeleoutText.x,g_xyTeleoutText.y, g_wszBuildables[3]);
		DrawLabel(g_xyTeleoutText.x,g_xyTeleoutText.y+16, g_wszNotBuilt);
	}
	else
	{
		if(g_iTeam == RED)
			g_CommonMask[2].DrawWhite(RED);
		else
			g_CommonMask[2].DrawWhite(BLU);

		DrawCommon(3);

		g_TeleoutLevelNumberIcon.DrawWhite(g_Build.teleout.level - 1);

		if(g_Build.teleout.flags & BUILD_BUILDING)
		{
			DrawLabel(g_xyTeleoutText.x, g_xyTeleoutText.y, g_wszBuilding);
			DrawBar(&g_TeleoutBuildingBar,(float)g_Build.teleout.progress/100.0f);
		}
		else
		{
			g_TeleoutProgressIcon.Draw();
			DrawBar(&g_TeleoutProgressBar,(float)g_Build.telein.charge/100.0f);

			if(g_Build.teleout.level != 3)
			{
				g_TeleoutUpgradeIcon.Draw();
				DrawBar(&g_TeleoutUpgradeBar,(float)g_Build.teleout.upgrade/200.0f);
			}
		}
	}
	g_TeleoutIcon.DrawWhite();
}

void DrawTelein(void)
{
	wchar_t wszTeleCount[8];
	if(g_Build.telein.level == 0)
	{
		g_CommonMask[1].DrawWhite(DISABLED);
		DrawLabel(g_xyTeleinText.x,g_xyTeleinText.y, g_wszBuildables[2]);
		DrawLabel(g_xyTeleinText.x,g_xyTeleinText.y+16, g_wszNotBuilt);
	}
	else
	{
		//±³¾°Ãæ°å
		if(g_iTeam == RED)
			g_CommonMask[1].DrawWhite(RED);
		else
			g_CommonMask[1].DrawWhite(BLU);

		DrawCommon(2);

		if(g_Build.telein.flags & BUILD_BUILDING)
		{
			DrawLabel(g_xyTeleinText.x, g_xyTeleinText.y, g_wszBuilding);
			DrawBar(&g_TeleinBuildingBar,(float)g_Build.telein.progress/100.0f);
		}
		else
		{
			g_TeleinLevelNumberIcon.DrawWhite(g_Build.telein.level - 1);
			g_TeleinCountIcon.Draw();
			wsprintfW(wszTeleCount, L"%d", g_Build.telein.frags);
			DrawLabel(g_xyTeleinCountText.x, g_xyTeleinCountText.y, wszTeleCount);

			if(g_Build.telein.level != 3)
			{
				g_TeleinUpgradeIcon.Draw();
				DrawBar(&g_TeleinUpgradeBar,(float)g_Build.telein.upgrade/200.0f);
			}
		}
	}
	g_TeleinIcon.DrawWhite();
}

void DrawDispenser(void)
{
	if(g_Build.dispenser.level == 0)
	{
		g_CommonMask[0].DrawWhite(DISABLED);
		DrawLabel(g_xyDispenserText.x, g_xyDispenserText.y, g_wszBuildables[1]);
		DrawLabel(g_xyDispenserText.x, g_xyDispenserText.y+16, g_wszNotBuilt);
	}
	else
	{
		//±³¾°Ãæ°å
		if(g_iTeam==RED)
			g_CommonMask[0].DrawWhite(RED);
		else
			g_CommonMask[0].DrawWhite(BLU);

		DrawCommon(1);

		if(g_Build.dispenser.flags & BUILD_BUILDING)
		{
			DrawLabel(g_xyDispenserText.x, g_xyDispenserText.y, g_wszBuilding);
			DrawBar(&g_DispenserBuildingBar,(float)g_Build.dispenser.progress/100.0f);
		}
		else
		{
			g_DispenserLevelNumberIcon.DrawWhite(g_Build.dispenser.level - 1);

			g_DispenserAmmoIcon.Draw();
			DrawBar(&g_DispenserAmmoBar,(float)g_Build.dispenser.metal/(float)g_Build.dispenser.maxmetal);

			if(g_Build.dispenser.level != 3)
			{
				g_DispenserUpgradeIcon.Draw();
				DrawBar(&g_DispenserUpgradeBar,(float)g_Build.dispenser.upgrade/200.0f);
			}
		}
	}
	g_DispenserIcon.DrawWhite();
}

void DrawSentry(void)
{
	static wchar_t wszKillCount[4];

	if(g_Build.sentry.level == 0)
	{
		g_SentryMask.DrawWhite(DISABLED);
		g_SentryIcon.DrawWhite(0);
		DrawLabel(g_xySentryDisabledText.x, g_xySentryDisabledText.y, g_wszBuildables[0]);
		DrawLabel(g_xySentryDisabledText.x, g_xySentryDisabledText.y+16, g_wszNotBuilt);
	}
	else
	{
		if(g_iTeam == RED)
			g_SentryMask.DrawWhite(RED);
		else
			g_SentryMask.DrawWhite(BLU);

		DrawCommon(0);

		if(g_Build.sentry.flags & BUILD_BUILDING)
		{
			DrawLabel(g_xySentryBuildingText.x, g_xySentryBuildingText.y, g_wszBuilding);
			DrawBar(&g_SentryBuildingBar,(float)g_Build.sentry.progress/100.0f);
			g_SentryIcon.DrawWhite(0);
		}
		else
		{
			int lvl = g_Build.sentry.level - 1;

			g_SentryIcon.DrawWhite(lvl);
			g_SentryLevelNumberIcon.DrawWhite(lvl);

			g_SentryKillIcon.Draw();
			wsprintfW(wszKillCount, L"%d", g_Build.sentry.killcount);
			DrawLabel(g_xySentryKillCountText.x,g_xySentryKillCountText.y, wszKillCount);

			g_SentryAmmoIcon.Draw();
			DrawBar(&g_SentryAmmoBar,(float)g_Build.sentry.ammo/(float)g_Build.sentry.maxammo);

			//Éý¼¶»ò»ð¼ý
			if(g_Build.sentry.level != 3)
			{
				g_SentryUpgradeIcon.Draw();
				DrawBar(&g_SentryUpgradeBar,(float)g_Build.sentry.upgrade/200.0f);
			}
			else
			{
				g_SentryRocketIcon.Draw();
				DrawBar(&g_SentryUpgradeBar,(float)g_Build.sentry.rocket/(float)g_Build.sentry.maxrocket);
			}
		}
	}
}