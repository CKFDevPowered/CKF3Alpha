#include "hud_base.h"
#include "client.h"
#include "cvar.h"
#include "util.h"

extern cl_entity_t *g_pTraceEntity;

static vgui::HFont g_hNameFont;

xy_t g_xyBarOffset;

void DrawHudMask(int col, int x, int y, int w, int h);
void DrawCross(int x, int y, float health, float scale);

static color24 g_ubColor[] = {{235,225,200}};

int HudStatusBar_VidInit(void)
{
	g_xyBarOffset.x = 0;
	g_xyBarOffset.y = .2 * ScreenHeight;

	g_hNameFont = g_pSurface->CreateFont();
	g_pSurface->AddGlyphSetToFont(g_hNameFont, "TF2 Secondary", 20 * ScreenHeight / 480, 0, 0, 0, vgui::ISurface::FONTFLAG_CUSTOM | vgui::ISurface::FONTFLAG_ANTIALIAS, 0x0, 0xFFFF);

	return 1;
}

static int CanSeeHealth(cl_entity_t *pEntity)
{
	if((g_iTeam == 1 || g_iTeam == 2) && pEntity->curstate.team == 3 - g_iTeam)
		return 0;

	if(g_PlayerInfo[pEntity->index].iTeam != g_iTeam)
		return 1;

	return 2;
}

int HudStatusBar_Redraw(float flTime, int iIntermission)
{
	if(iIntermission)
		return 0;

	int x, y, w, h;
	int xo, yo;
	wchar_t szText[32];

	cl_entity_t *pEntity = g_pTraceEntity;
	
	if(!pEntity)
		return 0;
	
	if(pEntity->player && pEntity->index >= 1 && pEntity->index <= gEngfuncs.GetMaxClients())
	{
		int iShowTeam = pEntity->curstate.team;
		int iShowClass = pEntity->curstate.playerclass;

		if(!CL_IsValidPlayer(iShowTeam, iShowClass))
			return 0;

		if(pEntity->curstate.renderfx == kRenderFxCloak && pEntity->curstate.renderamt < 255)
			return 0;

		int iShowHealth = pEntity->curstate.startpos[0];
		int iShowMaxHealth = pEntity->curstate.startpos[1];
		int iDisgTarget = pEntity->curstate.endpos[0];

		if(iDisgTarget)
		{
			iDisgTarget = (iDisgTarget>>1);
		}

		hud_player_info_t pi;
		gEngfuncs.pfnGetPlayerInfo((iDisgTarget) ? iDisgTarget : pEntity->index, &pi);

		if(!pi.name || !pi.name[0])
			return 0;

		int wt, ht;
		wcscpy(szText, ANSIToUnicode(pi.name));
		g_pSurface->GetTextSize(g_hNameFont, szText, wt, ht);

		w = .25 * ScreenHeight + wt;
		h = .1 * ScreenHeight;
		x = (ScreenWidth - w)/2 + g_xyBarOffset.x;
		y = (ScreenHeight - h)/2 + g_xyBarOffset.y;
		g_pSurface->DrawSetColor(255,255,255,200);
		if(g_iTeam == g_PlayerInfo[pEntity->index].iTeam)
		{
			DrawHudMask(g_iTeam, x, y, w, h);
		}
		else
		{
			DrawHudMask(iShowTeam, x, y, w, h);
		}

		xo = x + .02 * ScreenHeight;
		yo = y + .015 * ScreenHeight;
		if(CanSeeHealth(pEntity))
		{
			DrawCross(xo, yo, (float)iShowHealth / iShowMaxHealth, .25 * ScreenHeight / 480);
		}

		g_pSurface->DrawSetTextColor(g_ubColor[0].r,g_ubColor[0].g,g_ubColor[0].b,255);
		g_pSurface->DrawSetTextFont(g_hNameFont);
		g_pSurface->DrawSetTextPos(x+(w-wt)/2, y+(h-ht)/2);
		g_pSurface->DrawPrintText(szText, wcslen(szText));
	}
	return 1;
}