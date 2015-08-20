#include "hud.h"
#include "parsemsg.h"
#include "cl_util.h"
#include "client.h"
#include <com_model.h>
#include "configs.h"

float g_LocationColor[3] = { 0.0, 0.8, 0.0 };

#define RADAR_DOT_NORMAL 0
#define RADAR_DOT_BOMB (1<<0)
#define RADAR_DOT_HOSTAGE (1<<1)
#define RADAR_DOT_BOMBCARRIER (1<<2)
#define RADAR_DOT_BOMB_PLANTED (1<<3)

pfnUserMsgHook g_pfnMSG_Radar;

DECLARE_MESSAGE(m_Radar, Radar)
DECLARE_COMMAND(m_Radar, TrackPlayer);
DECLARE_COMMAND(m_Radar, ClearPlayers);
DECLARE_COMMAND(m_Radar, DrawRadar);
DECLARE_COMMAND(m_Radar, HideRadar);

int CHudRadar::Init(void)
{
	g_pfnMSG_Radar = HOOK_MESSAGE(Radar);

	HOOK_COMMAND("trackplayer", TrackPlayer);
	HOOK_COMMAND("clearplayers", ClearPlayers);
	HOOK_COMMAND("drawradar", DrawRadar);
	HOOK_COMMAND("hideradar", HideRadar);

	memset(m_bTrackArray, 0, sizeof(m_bTrackArray));

	m_iPlayerLastPointedAt = 0;
	m_bDrawRadar = true;

	gHUD.AddHudElem(this);
	return 1;
}

void CHudRadar::UserCmd_TrackPlayer(void)
{
	if (m_iPlayerLastPointedAt <= MAX_CLIENTS)
		m_bTrackArray[m_iPlayerLastPointedAt] = true;
}

void CHudRadar::UserCmd_ClearPlayers(void)
{
	memset(m_bTrackArray, 0, sizeof(m_bTrackArray));
}

void CHudRadar::UserCmd_DrawRadar(void)
{
	m_bDrawRadar = true;
}

void CHudRadar::UserCmd_HideRadar(void)
{
	m_bDrawRadar = false;
}

void CHudRadar::Reset(void)
{
}

int CHudRadar::VidInit(void)
{
	m_iFlags |= HUD_ACTIVE;

	m_HUD_radar = gHUD.GetSpriteIndex("radar");
	m_HUD_radaropaque = gHUD.GetSpriteIndex("radaropaque");

	m_hrad = &gHUD.GetSpriteRect(m_HUD_radar);
	m_hradopaque = &gHUD.GetSpriteRect(m_HUD_radaropaque);

	m_hrad->left = 0;
	m_hrad->top = 0;
	m_hrad->right = 128;
	m_hrad->bottom = 128;

	m_hradopaque->left = 0;
	m_hradopaque->top = 0;
	m_hradopaque->right = 128;
	m_hradopaque->bottom = 128;

	m_hRadar = gHUD.GetSprite(m_HUD_radar);
	m_hRadaropaque = gHUD.GetSprite(m_HUD_radaropaque);
	return 1;
}

int CHudRadar::MsgFunc_Radar(const char *pszName, int iSize, void *pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int cl = READ_BYTE();
	g_PlayerExtraInfo[cl].origin[0] = READ_COORD();
	g_PlayerExtraInfo[cl].origin[1] = READ_COORD();
	g_PlayerExtraInfo[cl].origin[2] = READ_COORD();

	return g_pfnMSG_Radar(pszName, iSize, pbuf);
}

int CHudRadar::Draw(float flTime)
{
	return 0;

	if ((gHUD.m_iHideHUDDisplay & HIDEHUD_HEALTH) || g_iUser1)
		return 1;

	if (!gHUD.m_fPlayerDead && m_bDrawRadar == true)
		DrawRadar(flTime);

	return 1;
}

void CHudRadar::DrawRadarDot(int x, int y, float z_diff, int iBaseDotSize, int flags, int r, int g, int b, int a)
{
	if (flags & RADAR_DOT_BOMBCARRIER)
	{
		gEngfuncs.pfnFillRGBA(x - 3, y - 3, iBaseDotSize + 3, iBaseDotSize + 3, r, g, b, a);
	}
	else if (z_diff < -128)
	{
		z_diff *= -1;

		if (z_diff > 3096)
			z_diff = 3096;

		int iBar = (int)(z_diff / 400) + 2 * iBaseDotSize;

		gEngfuncs.pfnFillRGBA(x, y, 1, iBar, r, g, b, a);
		gEngfuncs.pfnFillRGBA(x - 2, y + iBar, 5, 1, r, g, b, a);
	}
	else if (z_diff > 128)
	{
		if (z_diff > 3096)
			z_diff = 3096;

		int iBar = (int)(z_diff / 400) + 2 * iBaseDotSize;

		gEngfuncs.pfnFillRGBA(x - 2, y, 5, 1, r, g, b, a);
		gEngfuncs.pfnFillRGBA(x, y + 1, 1, iBar - 1, r, g, b, a);
	}
	else
	{
		if (flags & RADAR_DOT_HOSTAGE)
		{
			gEngfuncs.pfnFillRGBA(x - 1, y - 1, iBaseDotSize + 1, iBaseDotSize + 1, r, g, b, a);
		}
		else if (flags & RADAR_DOT_BOMB)
		{
			if (flags & RADAR_DOT_BOMB_PLANTED)
			{
				gEngfuncs.pfnFillRGBA(x, y, iBaseDotSize, iBaseDotSize, r, g, b, a);
				gEngfuncs.pfnFillRGBA(x - 2, y - 2, iBaseDotSize, iBaseDotSize, r, g, b, a);
				gEngfuncs.pfnFillRGBA(x - 2, y + 2, iBaseDotSize, iBaseDotSize, r, g, b, a);
				gEngfuncs.pfnFillRGBA(x + 2, y - 2, iBaseDotSize, iBaseDotSize, r, g, b, a);
				gEngfuncs.pfnFillRGBA(x + 2, y + 2, iBaseDotSize, iBaseDotSize, r, g, b, a);
			}
			else
			{
				gEngfuncs.pfnFillRGBA(x - 1, y - 1, iBaseDotSize + 1, iBaseDotSize + 1, r, g, b, a);
			}
		}
		else
		{
			gEngfuncs.pfnFillRGBA(x, y, iBaseDotSize, iBaseDotSize, r, g, b, a);
		}
	}
}

void CHudRadar::DrawRadar(float flTime)
{
	if (gConfigs.bEnableClientUI && cl_newradar->value)
		return;

	Vector vPlayer;
	char szTeamName[MAX_TEAM_NAME];
	float x_diff;
	float y_diff;
	float z_diff;
	float flOffset;
	float xnew_diff;
	float ynew_diff;
	float fRange;
	int iRadarRadius;
	int iBaseDotSize;
	int x, y, r, g, b;
	float flDelay;
	float flScale;

	vPlayer = gHUD.m_vecOrigin;
	strcpy(szTeamName, g_PlayerExtraInfo[gHUD.m_iPlayerNum].teamname);

	if (g_PlayerExtraInfo[gHUD.m_iPlayerNum].dead == true)
		return;

	if (cl_radartype && cl_radartype->value != 0)
	{
		SPR_Set(m_hRadaropaque, 200, 200, 200);
		SPR_DrawHoles(0, 0, 0, m_hradopaque);
	}
	else
	{
		SPR_Set(m_hRadar, 25, 75, 25);
		SPR_DrawAdditive(0, 0, 0, m_hrad);
	}

	for (int i = 0; i < MAX_CLIENTS + 1; i++)
	{
		if (i != 32 && (!g_PlayerInfoList[i].name || !g_PlayerInfoList[i].name[0]))
			continue;

		if (gHUD.m_iPlayerNum == i || strcmp(szTeamName, g_PlayerExtraInfo[i].teamname) || g_PlayerExtraInfo[i].dead)
			continue;

		x_diff = g_PlayerExtraInfo[i].origin[0] - vPlayer[0];
		y_diff = g_PlayerExtraInfo[i].origin[1] - vPlayer[1];
		z_diff = g_PlayerExtraInfo[i].origin[2] - vPlayer[2];

		flOffset = atan(y_diff / x_diff);
		flOffset *= 180;
		flOffset /= M_PI;

		if ((x_diff < 0) && (y_diff >= 0))
			flOffset = 180 + flOffset;
		else if ((x_diff < 0) && (y_diff < 0))
			flOffset = 180 + flOffset;
		else if ((x_diff >= 0) && (y_diff < 0))
			flOffset = 360 + flOffset;

		y_diff = -1 * sqrt((x_diff * x_diff) + (y_diff * y_diff));
		x_diff = 0;

		flOffset = gHUD.m_vecAngles[1] - flOffset;
		flOffset *= M_PI;
		flOffset /= 180;

		xnew_diff = x_diff * cos(flOffset) - y_diff * sin(flOffset);
		ynew_diff = x_diff * sin(flOffset) + y_diff * cos(flOffset);

		if (ScreenWidth >= 640)
		{
			fRange = 2048;
			iRadarRadius = 128;
			iBaseDotSize = 2;
		}
		else
		{
			fRange = 1024;
			iRadarRadius = 64;
			iBaseDotSize = 1;
		}

		if ((-1 * y_diff) > fRange)
		{
			flScale = (-1 * y_diff) / fRange;

			xnew_diff /= flScale;
			ynew_diff /= flScale;
		}

		xnew_diff /= 32;
		ynew_diff /= 32;

		x = (iRadarRadius / 2) + (int)xnew_diff;
		y = (iRadarRadius / 2) + (int)ynew_diff;

		if (x < 0 || x > iRadarRadius || y < 0 || y > iRadarRadius)
			continue;

		if ((g_PlayerExtraInfo[i].has_c4 && (g_iTeamNumber == TEAM_TERRORIST || g_iTeamNumber == TEAM_UNASSIGNED)) || (g_PlayerExtraInfo[i].vip && (g_iTeamNumber == TEAM_CT || g_iTeamNumber == TEAM_UNASSIGNED)))
		{
			r = 250;
			g = 0;
			b = 0;
		}
		else
		{
			if (m_bTrackArray[i] == true)
			{
				iBaseDotSize *= 2;
				r = 185;
				g = 20;
				b = 20;
			}
			else
			{
				r = 150;
				g = 75;
				b = 250;
			}
		}

		if (i != 32)
			DrawRadarDot(x, y, z_diff, iBaseDotSize, RADAR_DOT_NORMAL, r, g, b, 235);

		if (g_PlayerExtraInfo[i].radarflash != -1.0 && flTime > g_PlayerExtraInfo[i].radarflash && g_PlayerExtraInfo[i].radarflashes > 0)
		{
			if (i == 32)
			{
				flDelay = 0.35;

				if (g_PlayerExtraInfo[i].playerclass == 1)
					flDelay = 0.15;
			}
			else
			{
				flDelay = 0.15;
			}

			g_PlayerExtraInfo[i].radarflash = flTime + flDelay;
			g_PlayerExtraInfo[i].radarflashes--;
			g_PlayerExtraInfo[i].radarflashon = 1 - g_PlayerExtraInfo[i].radarflashon;
		}

		if (g_PlayerExtraInfo[i].radarflashon == 1 && g_PlayerExtraInfo[i].radarflashes > 0)
		{
			if (i == 32)
			{
				if (g_iTeamNumber == TEAM_TERRORIST)
				{
					r = 250;
					g = 0;
					b = 0;

					DrawRadarDot(x, y, z_diff, iBaseDotSize, (g_PlayerExtraInfo[i].playerclass == 1) ? (RADAR_DOT_BOMB | RADAR_DOT_BOMB_PLANTED) : RADAR_DOT_BOMB, r, g, b, 245);
				}
			}
			else
			{
				r = 230;
				g = 110;
				b = 25;

				DrawRadarDot(x, y, z_diff, iBaseDotSize, RADAR_DOT_BOMBCARRIER, r, g, b, 245);
			}
		}
	}

	for (int i = 0; i <= MAX_HOSTAGES; i++)
	{
		if ((strcmp(szTeamName, g_HostageInfo[i].teamname) || g_HostageInfo[i].dead) && (g_HostageInfo[i].dead != true || g_HostageInfo[i].radarflash == -1))
			continue;

		x_diff = g_HostageInfo[i].origin[0] - vPlayer[0];
		y_diff = g_HostageInfo[i].origin[1] - vPlayer[1];
		z_diff = g_HostageInfo[i].origin[2] - vPlayer[2];

		flOffset = atan(y_diff / x_diff);
		flOffset *= 180;
		flOffset /= M_PI;

		if ((x_diff < 0) && (y_diff >= 0))
			flOffset = 180 + flOffset;
		else if ((x_diff < 0) && (y_diff < 0))
			flOffset = 180 + flOffset;
		else if ((x_diff >= 0) && (y_diff < 0))
			flOffset = 360 + flOffset;

		y_diff = -1 * sqrt((x_diff * x_diff) + (y_diff * y_diff));
		x_diff = 0;

		flOffset = gHUD.m_vecAngles[1] - flOffset;
		flOffset *= M_PI;
		flOffset /= 180;

		xnew_diff = x_diff * cos(flOffset) - y_diff * sin(flOffset);
		ynew_diff = x_diff * sin(flOffset) + y_diff * cos(flOffset);

		if (ScreenWidth >= 640)
		{
			fRange = 2048;
			iRadarRadius = 128;
			iBaseDotSize = 2;
		}
		else
		{
			fRange = 1024;
			iRadarRadius = 64;
			iBaseDotSize = 1;
		}

		if ((-1 * y_diff) > fRange)
		{
			flScale = (-1 * y_diff) / fRange;

			xnew_diff /= flScale;
			ynew_diff /= flScale;
		}

		xnew_diff /= 32;
		ynew_diff /= 32;

		x = (iRadarRadius / 2) + (int)xnew_diff;
		y = (iRadarRadius / 2) + (int)ynew_diff;

		if (x < 0 || x > iRadarRadius || y < 0 || y > iRadarRadius)
			continue;

		if (g_HostageInfo[i].radarflash != -1.0 && flTime > g_HostageInfo[i].radarflash && g_HostageInfo[i].radarflashes > 0)
		{
			flDelay = 0.35;

			if (g_HostageInfo[i].dead == true)
				flDelay = 0.15;

			g_HostageInfo[i].radarflash = flTime + flDelay;
			g_HostageInfo[i].radarflashes--;
			g_HostageInfo[i].radarflashon = 1 - g_HostageInfo[i].radarflashon;
		}

		if (g_HostageInfo[i].radarflashon == 1 && g_HostageInfo[i].radarflashes > 0)
		{
			r = 50;
			g = 50;
			b = 255;

			if (g_HostageInfo[i].dead)
			{
				r = 250;
				g = 25;
				b = 25;
			}

			DrawRadarDot(x, y, z_diff, iBaseDotSize, RADAR_DOT_HOSTAGE, r, g, b, 245);
		}
	}

	DrawPlayerLocation();
}

int CHudRadar::GetRadarSize(void)
{
	if (cl_newradar->value && gConfigs.bEnableClientUI)
		return cl_newradar_size->value * ScreenWidth;

	if (ScreenWidth >= 640)
		return 128;
	else
		return 64;
}

void CHudRadar::DrawPlayerLocation(void)
{
	wchar_t *locString;
	int center_x, center_y;
	int string_width, string_height;
	int x, y;

	if (g_PlayerExtraInfo[gHUD.m_iPlayerNum].location[0] != '#')
	{
		static wchar_t locBuffer[512];
		vgui::localize()->ConvertANSIToUnicode(g_PlayerExtraInfo[gHUD.m_iPlayerNum].location, locBuffer, sizeof(locBuffer));
		locString = locBuffer;
	}
	else
	{
		locString = vgui::localize()->Find(g_PlayerExtraInfo[gHUD.m_iPlayerNum].location);
	}

	if (!locString)
		return;

	center_y = 128;
	center_x = 32;

	if (ScreenWidth < 640)
		center_y = 64;

	if (ScreenWidth >= 640)
		center_x = 64;

	gHUD.m_VGUI2Print.GetStringSize(locString, &string_width, &string_height);

	x = max(0, center_x - (string_width / 2));
	y = center_y + (string_height / 2);

	if (wcslen(locString) > 0)
		gHUD.m_VGUI2Print.DrawVGUI2String(locString, x, y, g_LocationColor[0], g_LocationColor[1], g_LocationColor[2]);
}