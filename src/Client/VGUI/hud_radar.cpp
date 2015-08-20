#include "qgl.h"
#include "hud.h"
#include "hud_radar.h"
#include "cl_util.h"
#include "parsemsg.h"
#include "com_model.h"
#include "player.h"
#include "client.h"
#include <triangleapi.h>
#include <cl_entity.h>
#include <ref_params.h>
#include <IEngineSurface.h>

#include <VGUI/VGUI.h>
#include <VGUI/ISurface.h>
#include <vgui_controls/controls.h>

#include "CounterStrikeViewport.h"
#include "Video.h"

#if 0
#define VGUI_DRAW
#endif

#if 1
#define USE_HOSTAGEENTITY
#endif

model_t *g_MapSprite;
extern vec3_t v_origin, v_angles;

namespace vgui
{
CHudRadar::CHudRadar(void) : Panel(NULL, "HudRadar")
{
	m_hHudFont = INVALID_FONT;
	m_bCanRenderMapSprite = false;
	m_iLastWide = 0;

	SetPaintBackgroundEnabled(false);
	SetMouseInputEnabled(false);
	SetKeyBoardInputEnabled(false);
}

void CHudRadar::Init(void)
{
	m_bCanRenderMapSprite = false;
	m_iLastWide = 0;

	vgui::HScheme hScheme = vgui::scheme()->GetDefaultScheme();
	vgui::IScheme *pScheme = vgui::scheme()->GetIScheme(hScheme);

	m_hHudFont = pScheme->GetFont("EngineFont", true);

	SetVisible(true);
}

void CHudRadar::VidInit(void)
{
	g_MapSprite = NULL;

	m_hsprPlayer = SPR_Load("sprites/iplayer.spr");
	m_hsprPlayerBlue = SPR_Load("sprites/iplayerblue.spr");
	m_hsprPlayerRed = SPR_Load("sprites/iplayerred.spr");
	m_hsprPlayerDead = SPR_Load("sprites/iplayerdead.spr");
	m_hsprPlayerC4 = SPR_Load("sprites/iplayerc4.spr");
	m_hsprPlayerVIP = SPR_Load("sprites/iplayervip.spr");
	m_hsprBackPack = SPR_Load("sprites/ibackpack.spr");
	m_hsprCamera = SPR_Load("sprites/camera.spr");
	m_hsprRadio = SPR_Load("sprites/iradio.spr");
	m_hsprC4 = SPR_Load("sprites/ic4.spr");
	m_hsprHostage = SPR_Load("sprites/ihostage.spr");

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
}

void CHudRadar::Reset(void)
{
	for (int i = 1; i <= MAX_HOSTAGES; i++)
		m_pHostages[i] = NULL;
}

void CHudRadar::Think(void)
{
	if (m_hHudFont == INVALID_FONT)
	{
		vgui::HScheme hScheme = vgui::scheme()->GetDefaultScheme();
		vgui::IScheme *pScheme = vgui::scheme()->GetIScheme(hScheme);

		m_hHudFont = pScheme->GetFont("EngineFont", true);
	}

	int wide = cl_newradar_size->value * ScreenWidth;

	if (wide == 0)
	{
		SetVisible(false);
	}
	else
	{
		if (m_iLastWide != wide)
			SetBounds(0, 0, wide, wide + vgui::surface()->GetFontTall(m_hHudFont) * 1.5);

		SetVisible(true);
	}
}

void CHudRadar::UpdateMapSprite(void)
{
	if (!g_MapSprite)
		return;
}

void CHudRadar::RenderMapSprite(void)
{
	if (!g_MapSprite)
		return; //

	if (!m_bCanRenderMapSprite)
		return;

	float screenaspect, xs, ys, xStep, yStep, x, y, z;
	int ix, iy, i, xTiles, yTiles, frame, numframes;

	i = g_MapSprite->numframes / (4 * 3);
	i = sqrt(float(i));

	xTiles = i * 4;
	yTiles = i * 3;

	screenaspect = 4.0f / 3.0f;
	xs = gHUD.m_Spectator.m_OverviewData.origin[0];
	ys = gHUD.m_Spectator.m_OverviewData.origin[1];
	z = v_origin[2] - (10 + 1.2 * 196.25) * 4.1f / gHUD.m_Spectator.m_OverviewData.zoom;

	gEngfuncs.pTriAPI->RenderMode(kRenderTransAlpha);
	gEngfuncs.pTriAPI->CullFace(TRI_NONE);
	gEngfuncs.pTriAPI->Color4f(cl_newradar_dark->value, cl_newradar_dark->value, cl_newradar_dark->value, cl_radartype->value ? 1.0f : 0.75f);

	frame = 0;
	numframes = g_MapSprite->numframes;

	if (gHUD.m_Spectator.m_OverviewData.rotated)
	{
		xStep = (2 * 4096.0f / gHUD.m_Spectator.m_OverviewData.zoom) / xTiles;
		yStep = -(2 * 4096.0f / (gHUD.m_Spectator.m_OverviewData.zoom * screenaspect)) / yTiles;
		y = ys + (4096.0f / (gHUD.m_Spectator.m_OverviewData.zoom * screenaspect));

		for (iy = 0; iy < yTiles; iy++)
		{
			x = xs - (4096.0f / (gHUD.m_Spectator.m_OverviewData.zoom));

			for (ix = 0; ix < xTiles; ix++)
			{
				if (frame >= numframes)
					break;

				gEngfuncs.pTriAPI->SpriteTexture(g_MapSprite, frame);
				gEngfuncs.pTriAPI->Begin(TRI_QUADS);
				gEngfuncs.pTriAPI->TexCoord2f(0, 0);
				gEngfuncs.pTriAPI->Vertex3f(x, y, z);
				gEngfuncs.pTriAPI->TexCoord2f(1, 0);
				gEngfuncs.pTriAPI->Vertex3f(x + xStep, y, z);
				gEngfuncs.pTriAPI->TexCoord2f(1, 1);
				gEngfuncs.pTriAPI->Vertex3f(x + xStep, y + yStep, z);
				gEngfuncs.pTriAPI->TexCoord2f(0, 1);
				gEngfuncs.pTriAPI->Vertex3f(x, y + yStep, z);
				gEngfuncs.pTriAPI->End();

				frame++;
				x += xStep;
			}

			y += yStep;
		}
	}
	else
	{
		xStep = -(2 * 4096.0f / gHUD.m_Spectator.m_OverviewData.zoom) / xTiles;
		yStep = -(2 * 4096.0f / (gHUD.m_Spectator.m_OverviewData.zoom * screenaspect)) / yTiles;
		x = xs + (4096.0f / (gHUD.m_Spectator.m_OverviewData.zoom * screenaspect));

		for (ix = 0; ix < yTiles; ix++)
		{
			y = ys + (4096.0f / (gHUD.m_Spectator.m_OverviewData.zoom));

			for (iy = 0; iy < xTiles; iy++)
			{
				if (frame >= numframes)
					break;

				gEngfuncs.pTriAPI->SpriteTexture(g_MapSprite, frame);
				gEngfuncs.pTriAPI->Begin(TRI_QUADS);
				gEngfuncs.pTriAPI->TexCoord2f(0, 0);
				gEngfuncs.pTriAPI->Vertex3f(x, y, z);
				gEngfuncs.pTriAPI->TexCoord2f(0, 1);
				gEngfuncs.pTriAPI->Vertex3f(x + xStep, y, z);
				gEngfuncs.pTriAPI->TexCoord2f(1, 1);
				gEngfuncs.pTriAPI->Vertex3f(x + xStep, y + yStep, z);
				gEngfuncs.pTriAPI->TexCoord2f(1, 0);
				gEngfuncs.pTriAPI->Vertex3f(x, y + yStep, z);
				gEngfuncs.pTriAPI->End();

				frame++;
				y += yStep;
			}

			x += xStep;
		}
	}
}

void CHudRadar::CalcRefdef(struct ref_params_s *pparams)
{
	m_bCanRenderMapSprite = false;

	if (g_iVideoMode != VIDEOMODE_D3D)
		return;

	if (!cl_newradar->value)
		return;

	if (!IsVisible())
		return;

	if (!g_MapSprite)
		return;

	if (!gHUD.m_pCvarDraw->value)
		return;

	if (gHUD.m_iIntermission || gEngfuncs.IsSpectateOnly())
		return;

	if ((gHUD.m_iHideHUDDisplay & HIDEHUD_HEALTH) || g_iUser1)
		return;

	if (!(!gHUD.m_fPlayerDead && gHUD.m_Radar.m_bDrawRadar))
		return;

	if (!(g_iTeamNumber == TEAM_CT || g_iTeamNumber == TEAM_TERRORIST))
		return;

	if (g_PlayerExtraInfo[gHUD.m_iPlayerNum].dead == true)
		return;

	if (!pparams->nextView)
	{
		pparams->nextView = 1;
		pparams->onlyClientDraw = false;
	}
	else
	{
		int sx, sy, wide, tall;

		GetBounds(sx, sy, wide, tall);
		tall = wide;

		pparams->viewport[0] = pparams->viewport[1] = 1;
		pparams->viewport[2] = wide - 2;
		pparams->viewport[3] = tall - 2;
		pparams->nextView = 0;
		pparams->onlyClientDraw = true;
		pparams->viewangles[0] = 90;
	}

	if (pparams->onlyClientDraw)
		m_bCanRenderMapSprite = true;
}

void CHudRadar::Paint(void)
{
	return;

	if (!cl_newradar->value)
		return;

	if (!g_MapSprite)
		return;

	if (!gHUD.m_pCvarDraw->value)
		return;

	if (gHUD.m_iIntermission || gEngfuncs.IsSpectateOnly())
		return;

	if ((gHUD.m_iHideHUDDisplay & HIDEHUD_HEALTH) || g_iUser1)
		return;

	if (!(!gHUD.m_fPlayerDead && gHUD.m_Radar.m_bDrawRadar))
		return;

	if (!(g_iTeamNumber == TEAM_CT || g_iTeamNumber == TEAM_TERRORIST))
		return;

	if (g_PlayerExtraInfo[gHUD.m_iPlayerNum].dead == true)
		return;

#ifdef USE_HOSTAGEENTITY
	if (gHUD.m_flTime > m_flNextBuild)
	{
		BuildHostageList();
		m_flNextBuild = gHUD.m_flTime + 1.0;
	}
#endif

	int sx, sy, wide, tall;
	GetBounds(sx, sy, wide, tall);
	tall = wide;

	if (g_iVideoMode != VIDEOMODE_D3D)
	{
		float angles, xTemp, yTemp, viewzoom;
		float screenaspect, xs, ys, xStep, yStep, x, y, z;
		int ix, iy, i, xTiles, yTiles, frame, numframes;
		float xUpStep, yUpStep, xRightStep, yRightStep;
		float xIn, yIn, xOut, yOut;

		viewzoom = 10.0;

		i = g_MapSprite->numframes / (4 * 3);
		i = sqrt(float(i));

		xTiles = i * 4;
		yTiles = i * 3;

		screenaspect = 4.0f / 3.0f;
		angles = (v_angles[1] + 90.0) * (M_PI / 180.0);
		xs = gHUD.m_Spectator.m_OverviewData.origin[0];
		ys = gHUD.m_Spectator.m_OverviewData.origin[1];
		z = 0;

		frame = 0;
		numframes = g_MapSprite->numframes;

		if (gHUD.m_Spectator.m_OverviewData.rotated)
		{
			angles -= M_PI / 2.0;
			xTemp = 3 + gHUD.m_Spectator.m_OverviewData.zoom * (1.0 / 1024.0) * ys - (1.0 / 1024) * gHUD.m_Spectator.m_OverviewData.zoom * g_vecEyePos[1];
			yTemp = -(-4 + gHUD.m_Spectator.m_OverviewData.zoom * (1.0 / 1024.0) * xs - (1.0 / 1024) * gHUD.m_Spectator.m_OverviewData.zoom * g_vecEyePos[0]);
		}
		else
		{
			xTemp = 3 + gHUD.m_Spectator.m_OverviewData.zoom * (1.0 / 1024.0) * xs - (1.0 / 1024) * gHUD.m_Spectator.m_OverviewData.zoom * g_vecEyePos[0];
			yTemp = 4 + gHUD.m_Spectator.m_OverviewData.zoom * (1.0 / 1024.0) * ys - (1.0 / 1024) * gHUD.m_Spectator.m_OverviewData.zoom * g_vecEyePos[1];
		}

		xStep = (2 * 4096.0f / viewzoom) / xTiles;
		yStep = -(2 * 4096.0f / (viewzoom * screenaspect)) / yTiles;

		xUpStep = cos(angles + (M_PI / 2)) * yStep;
		yUpStep = sin(angles + (M_PI / 2)) * yStep;
		xRightStep = cos(angles) * xStep;
		yRightStep = sin(angles) * xStep;

		xOut = wide * 0.5 - (xTemp * xRightStep) - (yTemp * xUpStep);
		yOut = tall * 0.5 - (xTemp * yRightStep) - (yTemp * yUpStep);

#ifndef VGUI_DRAW
		qglScissor(sx, ScreenHeight - tall, wide, ScreenHeight - sy);
		qglEnable(GL_SCISSOR_TEST);

		gEngfuncs.pTriAPI->RenderMode(kRenderTransAlpha);
		gEngfuncs.pTriAPI->CullFace(TRI_NONE);
		gEngfuncs.pTriAPI->Color4f(cl_newradar_dark->value, cl_newradar_dark->value, cl_newradar_dark->value, cl_radartype->value ? 1.0f : 0.75f);
#else
		vgui::surface()->DrawSetColor(255, 255, 255, cl_radartype->value ? 255 : 190);
#endif
		for (ix = 0; ix < yTiles; ix++)
		{
			xIn = xOut;
			yIn = yOut;

			for (iy = 0; iy < xTiles; iy++)
			{
				x = xIn;
				y = yIn;

				if (frame >= numframes)
					break;

#ifdef VGUI_DRAW
				int p[4][2];
				vgui::Vertex_t vert[4];

				vert[0].m_Position.x = x;
				vert[0].m_Position.y = y;
				vert[0].m_TexCoord.x = 0;
				vert[0].m_TexCoord.y = 0;

				vert[1].m_Position.x = x + xRightStep;
				vert[1].m_Position.y = y + yRightStep;
				vert[1].m_TexCoord.x = 0;
				vert[1].m_TexCoord.y = 1;

				vert[2].m_Position.x = x + xRightStep + xUpStep;
				vert[2].m_Position.y = y + yRightStep + yUpStep;
				vert[2].m_TexCoord.x = 1;
				vert[2].m_TexCoord.y = 1;

				vert[3].m_Position.x = x + xUpStep;
				vert[3].m_Position.y = y + yUpStep;
				vert[3].m_TexCoord.x = 1;
				vert[3].m_TexCoord.y = 0;

				vgui::surface()->DrawSetTexture(m_iTextures[frame]);
				vgui::surface()->DrawTexturedPolygon(4, vert);
#else
				gEngfuncs.pTriAPI->SpriteTexture(g_MapSprite, frame);

				gEngfuncs.pTriAPI->Begin(TRI_QUADS);
				gEngfuncs.pTriAPI->TexCoord2f(0, 0);
				gEngfuncs.pTriAPI->Vertex3f(x, y, 0.0);
				gEngfuncs.pTriAPI->TexCoord2f(0, 1);
				gEngfuncs.pTriAPI->Vertex3f(x + xRightStep, y + yRightStep, 0.0);
				gEngfuncs.pTriAPI->TexCoord2f(1, 1);
				gEngfuncs.pTriAPI->Vertex3f(x + xRightStep + xUpStep, y + yRightStep + yUpStep, 0.0);
				gEngfuncs.pTriAPI->TexCoord2f(1, 0);
				gEngfuncs.pTriAPI->Vertex3f(x + xUpStep, y + yUpStep, 0.0);
				gEngfuncs.pTriAPI->End();
#endif
				frame++;

				xIn += xUpStep;
				yIn += yUpStep;
			}

			xOut += xRightStep;
			yOut += yRightStep;
		}
#ifndef VGUI_DRAW
		qglDisable(GL_SCISSOR_TEST);
#endif
	}

	vgui::surface()->DrawSetColor(0, 0, 0, 255);
	vgui::surface()->DrawOutlinedRect(0, 0, wide, tall);

	if (ScreenWidth > 640)
	{
		wchar_t *locString;
		int center_x, center_y;
		int string_width, string_height;
		int x, y;

		locString = vgui::localize()->Find(g_PlayerExtraInfo[gHUD.m_iPlayerNum].location);

		if (!locString)
		{
			static wchar_t staticLoc[32];
			vgui::localize()->ConvertANSIToUnicode(g_PlayerExtraInfo[gHUD.m_iPlayerNum].location, staticLoc, sizeof(staticLoc));
			locString = staticLoc + 1;
		}

		if (locString)
		{
			center_y = tall;
			center_x = wide / 2;

			vgui::surface()->GetTextSize(m_hHudFont, locString, string_width, string_height);

			x = max(0, center_x - (string_width / 2));
			y = center_y + (string_height / 2);

			vgui::surface()->DrawSetTextFont(m_hHudFont);
			vgui::surface()->DrawSetTextPos(x, y);
			vgui::surface()->DrawSetTextColor(0, 204, 0, 255);
			vgui::surface()->DrawPrintText(locString, wcslen(locString));
			vgui::surface()->DrawFlushText();
		}
	}

	gEngfuncs.pTriAPI->RenderMode(kRenderTransAdd);
	gEngfuncs.pTriAPI->Color4f(1, 0.62745f, 0, 1.0f);

	struct model_s *model = (struct model_s *)gEngfuncs.GetSpritePointer(m_hsprCamera);
	gEngfuncs.pTriAPI->SpriteTexture(model, 0);

	float cameraScale = 2;
	int cameraWide = gEngfuncs.pfnSPR_Width(m_hsprCamera, 0) * cameraScale;
	int cameraHeight = gEngfuncs.pfnSPR_Height(m_hsprCamera, 0) * cameraScale;

	gEngfuncs.pTriAPI->Begin(TRI_TRIANGLES);
	gEngfuncs.pTriAPI->TexCoord2f(1, 1);
	gEngfuncs.pTriAPI->Vertex3f(wide / 2 + cameraWide * 0.7, tall / 2 - cameraHeight * 0.7, 0);
	gEngfuncs.pTriAPI->TexCoord2f(0, 0);
	gEngfuncs.pTriAPI->Vertex3f(wide / 2 - cameraWide * 0.7, tall / 2 - cameraHeight * 0.7, 0);
	gEngfuncs.pTriAPI->TexCoord2f(0, 1);
	gEngfuncs.pTriAPI->Vertex3f(wide / 2, tall / 2, 0);
	gEngfuncs.pTriAPI->End();

	gEngfuncs.pTriAPI->RenderMode(kRenderTransAlpha);

	float flTime = gHUD.m_flTime;
	float flDelay;

	char szTeamName[MAX_TEAM_NAME];
	strcpy(szTeamName, g_PlayerExtraInfo[gHUD.m_iPlayerNum].teamname);

	for (int i = 0; i < MAX_CLIENTS + 1; i++)
	{
		if (i != 32 && (!g_PlayerInfoList[i].name || !g_PlayerInfoList[i].name[0]))
			continue;

		if (strcmp(szTeamName, g_PlayerExtraInfo[i].teamname) || g_PlayerExtraInfo[i].dead)
			continue;

		int r, g, b;
		HSPRITE hspr = 0;
		HSPRITE hspr2 = 0;
		int scale = 8;
		float scale2 = 0.75;

		if (i == gHUD.m_iPlayerNum)
		{
		}
		else
		{
			if (g_PlayerExtraInfo[i].teamnumber == g_iTeamNumber)
			{
				if (g_iTeamNumber == TEAM_TERRORIST)
				{
					r = 255;
					g = 255;
					b = 255;
					hspr = m_hsprPlayerRed;

					if (g_PlayerExtraInfo[i].has_c4)
						hspr = m_hsprPlayerC4;
				}
				else if (g_iTeamNumber == TEAM_CT)
				{
					r = 255;
					g = 255;
					b = 255;
					hspr = m_hsprPlayerBlue;

					if (g_PlayerExtraInfo[i].vip)
						hspr = m_hsprPlayerVIP;
				}
			}
		}

		int rx, ry;
		float yaw = 0;

		if (i != 32)
		{
			if (hspr == 0)
				continue;

			cl_entity_t *ent = gEngfuncs.GetEntityByIndex(i);
			vec3_t *origin;
			bool valid;

			if (!IsValidEntity(ent))
			{
				valid = false;
				origin = &g_PlayerExtraInfo[i].origin;
			}
			else
			{
				valid = true;
				origin = &Vector(ent->origin);
			}

			if (!CalcPoint(*origin, rx, ry, scale))
			{
				if (valid)
					yaw = v_angles[1] - ent->angles[1];
			}

			DrawSprite(rx, ry, hspr, yaw, scale, r, g, b, 255);

			if (hspr2 != 0)
				DrawSprite(rx, ry, hspr2, yaw, scale * scale2, r, g, b, 255);
		}

		if (g_PlayerExtraInfo[i].radarflash != -1.0 && flTime > g_PlayerExtraInfo[i].radarflash && g_PlayerExtraInfo[i].radarflashes > 0)
		{
			if (i == 32)
			{
				flDelay = 0.35;

				if (g_PlayerExtraInfo[i].playerclass == 1)
					flDelay = 0.15;
			}
			else
				flDelay = 0.15;

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
					r = 255;
					g = 255;
					b = 255;
					hspr = m_hsprBackPack;
					scale = scale * 1.5;

					if (g_PlayerExtraInfo[i].playerclass == 1)
						hspr = m_hsprC4;

					CalcPoint(g_PlayerExtraInfo[i].origin, rx, ry, scale);
					DrawSprite(rx, ry, hspr, yaw, scale, r, g, b, 255);
				}
			}
			else
			{
				r = 255;
				g = 255;
				b = 255;
				hspr = m_hsprRadio;

				cl_entity_t *ent = gEngfuncs.GetEntityByIndex(i);
				vec3_t *origin;
				bool valid;

				if (!IsValidEntity(ent))
				{
					valid = false;
					origin = &g_PlayerExtraInfo[i].origin;
				}
				else
				{
					valid = true;
					origin = &Vector(ent->origin);
				}

				if (!CalcPoint(*origin, rx, ry, scale))
				{
					if (valid)
						yaw = v_angles[1] - ent->angles[1];
				}

				DrawSprite(rx, ry, hspr, yaw, scale, r, g, b, 255);
			}
		}
	}

	for (int i = 0; i <= MAX_HOSTAGES; i++)
	{
		if ((strcmp(szTeamName, g_HostageInfo[i].teamname) || g_HostageInfo[i].dead) && (g_HostageInfo[i].dead != true || g_HostageInfo[i].radarflash == -1))
			continue;

		int r, g, b;
		HSPRITE hspr = 0;
		HSPRITE hspr2 = 0;
		int scale = 8;
		int rx, ry;
		float yaw = 0;

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
			r = 255;
			g = 255;
			b = 255;
			hspr = m_hsprHostage;

			if (g_HostageInfo[i].dead)
			{
				r = 255;
				g = 255;
				b = 255;
			}

			cl_entity_t *ent = m_pHostages[i];
			vec3_t *origin;
			bool valid;

			if (!IsValidEntity(ent))
			{
				valid = false;
				origin = &g_HostageInfo[i].origin;
			}
			else
			{
				valid = true;
				origin = &Vector(ent->origin);
			}

			if (!CalcPoint(*origin, rx, ry, scale))
			{
				if (valid)
					yaw = v_angles[1] - ent->angles[1];
			}

			DrawSprite(rx, ry, hspr, yaw, scale, r, g, b, 255);
		}
	}

	gEngfuncs.pTriAPI->RenderMode(kRenderNormal);
}

void CHudRadar::BuildHostageList(void)
{
	int index = 1;

	for (int i = 1; ; i++)
	{
		cl_entity_t *entity = gEngfuncs.GetEntityByIndex(i);

		if (!entity)
			break;

		model_t *model = entity->model;

		if (model)
		{
			if (model->name[0] == '*')
				continue;

			if (!strcmp(model->name, "models/hostage.mdl"))
			{
				m_pHostages[index] = entity;
				index++;
			}
		}
	}

	for (int i = index; i <= MAX_HOSTAGES; i++)
		m_pHostages[i] = NULL;
}

bool CHudRadar::IsValidEntity(cl_entity_s *pEntity)
{
	if (pEntity && pEntity->model && pEntity->model->name && !(pEntity->curstate.messagenum < gEngfuncs.GetLocalPlayer()->curstate.messagenum))
		return true;

	return false;
}

#pragma optimize("", off)

bool CHudRadar::CalcPoint(float *origin, int &screenX, int &screenY, int &scale)
{
	int wide, tall;
	GetSize(wide, tall);

	tall = wide;

	float dx = origin[0] - g_vecEyePos[0];
	float dy = origin[1] - g_vecEyePos[1];

	float yaw = v_angles[1] * (M_PI / 180.0);

	float yawSin = sin(yaw);
	float yawCos = cos(yaw);

	float x = dx * yawSin - dy * yawCos;
	float y = dx * (-yawCos) - dy * yawSin;

	float zoom = 10;
	bool scaled = false;

	screenX = (wide / 2) + (x / zoom) * gHUD.m_Spectator.m_OverviewData.zoom;
	screenY = (tall / 2) + (y / zoom) * gHUD.m_Spectator.m_OverviewData.zoom;

	bool result = false;
	float scalleRate = (1 / 1.2);

	if (screenX < 0 + scale)
	{
		screenX = 0;
		result = true;
		scale *= scalleRate;
		scaled = true;
		screenX += scale + 1;
	}
	else if (screenX >= wide - scale)
	{
		screenX = wide;
		result = true;
		scale *= scalleRate;
		scaled = true;
		screenX -= scale + 1;
	}

	if (screenY < 0 + scale)
	{
		screenY = 0;
		result = true;

		if (!scaled)
			scale *= scalleRate;

		screenY += scale + 1;
	}
	else if (screenY >= tall - scale)
	{
		screenY = wide;
		result = true;

		if (!scaled)
			scale *= scalleRate;

		screenY -= scale + 1;
	}

	return result;
}

#pragma optimize("", on)

void CHudRadar::DrawSprite(int x, int y, HSPRITE hspr, float yaw, int scale, int r, int g, int b, int a)
{
	if (hspr == 0)
	{
		gEngfuncs.pfnFillRGBA(x - 10, y - 10, 20, 20, r, g, b, a);
		return;
	}

	gEngfuncs.pTriAPI->CullFace(TRI_NONE);
	gEngfuncs.pTriAPI->Color4ub(r, g, b, a);

	struct model_s *model = (struct model_s *)gEngfuncs.GetSpritePointer(hspr);
	gEngfuncs.pTriAPI->SpriteTexture(model, 0);

	vec3_t forward, right, sub;
	sub[0] = sub[2] = 0;
	sub[1] = yaw - 90.0;

	gEngfuncs.pfnAngleVectors(sub, forward, right, NULL);

	gEngfuncs.pTriAPI->Begin(TRI_QUADS);
	gEngfuncs.pTriAPI->TexCoord2f(1, 0);
	gEngfuncs.pTriAPI->Vertex3f(x + right.x * scale + forward.x * scale, y + right.y * scale + forward.y * scale, 0);
	gEngfuncs.pTriAPI->TexCoord2f(0, 0);
	gEngfuncs.pTriAPI->Vertex3f(x + right.x * scale + forward.x * -scale, y + right.y * scale + forward.y * -scale, 0);
	gEngfuncs.pTriAPI->TexCoord2f(0, 1);
	gEngfuncs.pTriAPI->Vertex3f(x + right.x * -scale + forward.x * -scale, y + right.y * -scale + forward.y * -scale, 0);
	gEngfuncs.pTriAPI->TexCoord2f(1, 1);
	gEngfuncs.pTriAPI->Vertex3f(x + right.x * -scale + forward.x * scale, y + right.y * -scale + forward.y * scale, 0);
	gEngfuncs.pTriAPI->End();
}

void CHudRadar::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

int CHudRadar::FireMessage(const char *pszName, int iSize, void *pbuf)
{
	return 0;
}
}