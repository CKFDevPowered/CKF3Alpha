#include "hud_base.h"
#include "exportfuncs.h"
#include "client.h"

void DrawCross(int x, int y, float health, float scale);

vgui::HFont g_hHealthFont;
int g_texHealthBg;
int g_texHealthColor;

int HudHealth_VidInit(void)
{
	g_texHealthBg = Surface_LoadTGA("resource\\tga\\health_bg");
	g_texHealthColor = Surface_LoadTGA("resource\\tga\\health_Color");

	g_hHealthFont = g_pSurface->CreateFont();
	g_pSurface->AddGlyphSetToFont(g_hHealthFont, "TF2", 20 * ScreenHeight / 480, 0, 0, 0, vgui::ISurface::FONTFLAG_CUSTOM | vgui::ISurface::FONTFLAG_ANTIALIAS, 0x48, 0x57);//字体大小36, 自带抗锯齿
	return 1;
}

int HudHealth_Redraw(float flTime, int iIntermission)
{
	if(iIntermission)
		return 0;

	if((g_iHideHUD & HIDEHUD_HEALTH))
		return 0;

	if(gEngfuncs.IsSpectateOnly())
		return 0;

	if(g_iMaxHealth <= 0 || g_iHealth <= 0)
		return 0;

	int x, y, w, h, w2, h2;
	float scale640;

	x = 73*(ScreenHeight*4/3)/640;
	y = (480 - 87)*ScreenHeight/480;
	g_pSurface->DrawGetTextureSize(g_texHealthBg, w, h);
	scale640 = (ScreenHeight*55.0f/h)/480;

	DrawCross(x, y, (float)g_iHealth / g_iMaxHealth, scale640);

	wchar_t wszText[6];
	wsprintfW(wszText, L"%d", g_iHealth);
	g_pSurface->DrawSetTextColor(114, 108, 96, 255);

	g_pSurface->GetTextSize(g_hHealthFont, wszText, w2, h2);

	g_pSurface->DrawSetTextPos(x + (w * scale640 - w2) / 2, y + 38 * scale640);
	g_pSurface->DrawSetTextFont(g_hHealthFont);
	g_pSurface->DrawPrintText(wszText, wcslen(wszText));
	g_pSurface->DrawFlushText();
	return 1;
}

void DrawCross(int x, int y, float health, float scale)
{
	float flsDelta, flsOffset;

	int iHealthX, iHealthY, iOverHealthX, iOverHealthY, iBottomY;
	int r, g, b, a, w, h;
	qboolean fOverHealth;
	float flHealthScale, flOverScale;

	iHealthX = x;
	iHealthY = y;
	flHealthScale = scale; //0.6
	flOverScale = 0;
	fOverHealth = FALSE;
	r = g = b = a = 255;

	//calc delta

	float flTime = g_flClientTime * 2.0;

	flsDelta = (float)flTime - (float)((int)flTime);
	flsOffset = fabs(flsDelta - 0.5f) / 2.0f;

	qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	g_pSurface->DrawSetTexture(g_texHealthBg);
	g_pSurface->DrawSetColor(255, 255, 255, 255);
	g_pSurface->DrawGetTextureSize(g_texHealthBg, w, h);
	g_pSurface->DrawTexturedRect(iHealthX, iHealthY, iHealthX+w*flHealthScale, iHealthY+h*flHealthScale);

	if(health > 1.0f)
	{
		r = g = b = a = 255 * flsOffset * 4;
		flOverScale = health * flHealthScale;
		fOverHealth = TRUE;
	}
	else if(health <= 0.3f)
	{
		r = a = 255 * flsOffset * 4;
		g = b = 100 * flsOffset * 4;
		flOverScale = (1.3f - health) * flHealthScale;
		fOverHealth = TRUE;
	}

	g_pSurface->DrawGetTextureSize(g_texHealthBg, w, h);

	if(fOverHealth)
	{
		iOverHealthX = iHealthX + w * flHealthScale / 2;
		iOverHealthY = iHealthY + h * flHealthScale / 2;

		qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		g_pSurface->DrawSetTexture(g_texHealthBg);
		g_pSurface->DrawSetColor(r, g, b, a);

		g_pSurface->DrawTexturedRect(iOverHealthX-w*flOverScale/2, iOverHealthY-h*flOverScale/2, iOverHealthX+w*flOverScale/2, iOverHealthY+h*flOverScale/2);
	}

	iBottomY = iHealthY + h * flHealthScale;

	iHealthY += h * flHealthScale * (1 - min(health, 1));

	if(health < 0.4f)
	{
		r = 255;
		g = b = 100;
	}

	a = 200;

	g_pSurface->DrawGetTextureSize(g_texHealthColor, w, h);

	qglTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	qglEnable(GL_TEXTURE_2D);
	qglEnable(GL_BLEND);
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	qglBindTexture(GL_TEXTURE_2D, g_texHealthColor);

	//base

	qglColor4ub(60, 60, 70, 255);

	qglBegin(GL_QUADS);

	qglTexCoord2f(0,0);
	qglVertex3f(iHealthX + 2, iBottomY - h * flHealthScale + 2,0);

	qglTexCoord2f(0,1);
	qglVertex3f(iHealthX + 2, iBottomY - 2,0);

	qglTexCoord2f(1,1);
	qglVertex3f(iHealthX + w * flHealthScale - 2,iBottomY - 2,0);
		
	qglTexCoord2f(1,0);
	qglVertex3f(iHealthX + w * flHealthScale - 2,iBottomY - h * flHealthScale + 2,0);

	qglEnd();

	//overlay
	qglColor4ub(r, g, b, a);

	qglBegin(GL_QUADS);

	qglTexCoord2f(0,1-min(health, 1));
	qglVertex3f(iHealthX+2,iHealthY+2,0);

	qglTexCoord2f(0,1);
	qglVertex3f(iHealthX+2,iBottomY-2,0);

	qglTexCoord2f(1,1);
	qglVertex3f(iHealthX + w * flHealthScale - 2,iBottomY - 2,0);
		
	qglTexCoord2f(1,1-min(health, 1));
	qglVertex3f(iHealthX + w * flHealthScale - 2,iHealthY + 2,0);

	qglEnd();
}