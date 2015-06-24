#include "hud_base.h"
#include "exportfuncs.h"

//button
static vgui::HFont g_hButtonFont;
static int g_texButtonEdge;
static int g_sizeButtonEdge;

//hud menu
static int g_texHudMenuEdge;
static int g_texHudMenuSide;
static int g_sizeHudMenuEdge;

//hud menu key
static vgui::HFont g_hHudMenuKeyFont;
static int g_texHudMenuKey;

//hud status bar
static int g_texHudMaskEdge[3];
static int g_texHudMaskSide[3];
static int g_sizeHudMaskEdge;

//hit damage text
static vgui::HFont g_hHitDamageFont;
int g_texFloatText;

int HudCommon_VidInit(void)
{
	g_texButtonEdge = Surface_LoadTGA("resource\\tga\\btn_edge");

	g_hButtonFont = g_pSurface->CreateFont();
	g_pSurface->AddGlyphSetToFont(g_hButtonFont, "TF2", 16 * ScreenHeight / 480, 0, 0, 0, vgui::ISurface::FONTFLAG_CUSTOM | vgui::ISurface::FONTFLAG_ANTIALIAS, 0x0, 0xFFFF);
	g_sizeButtonEdge = 6 * ScreenHeight / 480;

	g_texHudMenuEdge = Surface_LoadTGA("resource\\tga\\hud_menuedge");
	g_texHudMenuSide = Surface_LoadTGA("resource\\tga\\hud_menuside");
	g_sizeHudMenuEdge = 9 * ScreenHeight / 480;

	g_hHudMenuKeyFont = g_pSurface->CreateFont();
	g_pSurface->AddGlyphSetToFont(g_hHudMenuKeyFont, "TF2", 11 * ScreenHeight / 480, 0, 0, 0, vgui::ISurface::FONTFLAG_CUSTOM | vgui::ISurface::FONTFLAG_ANTIALIAS, 0x0, 0xFFFF);
	g_texHudMenuKey = Surface_LoadTGA("resource\\tga\\ico_key_blank");

	g_texHudMaskEdge[0] = Surface_LoadTGA("resource\\tga\\mask_edged");
	g_texHudMaskSide[0] = Surface_LoadTGA("resource\\tga\\mask_sided");
	g_texHudMaskEdge[1] = Surface_LoadTGA("resource\\tga\\mask_edger");
	g_texHudMaskSide[1] = Surface_LoadTGA("resource\\tga\\mask_sider");
	g_texHudMaskEdge[2] = Surface_LoadTGA("resource\\tga\\mask_edgeb");
	g_texHudMaskSide[2] = Surface_LoadTGA("resource\\tga\\mask_sideb");
	g_sizeHudMaskEdge = 5 * ScreenHeight / 480;

	g_hHitDamageFont = g_pSurface->CreateFont();
	g_pSurface->AddGlyphSetToFont(g_hHitDamageFont, "TF2", 192, 0, 0, 0, vgui::ISurface::FONTFLAG_CUSTOM | vgui::ISurface::FONTFLAG_ANTIALIAS, 0x0, 0xFFFF);

	g_texFloatText = gpRefExports->R_GLGenTexture(ScreenWidth, ScreenHeight);

	return 1;
}

void DrawPointsQuad(xy_t *p)
{
	qglEnable(GL_TEXTURE_2D);
	qglEnable(GL_BLEND);
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	qglBegin(GL_QUADS);
	qglTexCoord2f(0,0);
	qglVertex3f(p->x,p->y,0);
	p ++;
	qglTexCoord2f(1,0);
	qglVertex3f(p->x,p->y,0);
	p ++;
	qglTexCoord2f(1,1);
	qglVertex3f(p->x,p->y,0);
	p ++;
	qglTexCoord2f(0,1);
	qglVertex3f(p->x,p->y,0);
	qglEnd();
}

void DrawTexturedEdge(int x, int y, int w, int h, int r)
{
	//draw 4 edges
	g_pSurface->DrawTexturedRect(x, y, x+r, y+r);
	g_pSurface->DrawTexturedRect(x+w, y, x+w-r, y+r);
	g_pSurface->DrawTexturedRect(x+w, y+h, x+w-r, y+h-r);
	g_pSurface->DrawTexturedRect(x, y+h, x+r, y+h-r);
}

void DrawFilledEdge(int x, int y, int w, int h, int r)
{
	//draw 4 edges
	g_pSurface->DrawFilledRect(x, y, x+r, y+r);
	g_pSurface->DrawFilledRect(x+w, y, x+w-r, y+r);
	g_pSurface->DrawFilledRect(x+w, y+h, x+w-r, y+h-r);
	g_pSurface->DrawFilledRect(x, y+h, x+r, y+h-r);
}

void DrawTexturedSide(int x, int y, int w, int h, int r)
{
	//fill 4 sides, bug fixed
	xy_t p[4];
	g_pSurface->DrawTexturedRect(x+r, y, x+w-r, y+r);
	p[0].x = x+w;p[0].y = y+r;
	p[1].x = x+w;p[1].y = y+h-r;
	p[2].x = x+w-r;p[2].y = y+h-r;
	p[3].x = x+w-r;p[3].y = y+r;
	DrawPointsQuad(p);
	g_pSurface->DrawTexturedRect(x+w-r, y+h, x+r, y+h-r);
	p[0].x = x;p[0].y = y+h-r;
	p[1].x = x;p[1].y = y+r;
	p[2].x = x+r;p[2].y = y+r;
	p[3].x = x+r;p[3].y = y+h-r;
	DrawPointsQuad(p);
}

void DrawFilledSide(int x, int y, int w, int h, int r)
{
	//fill 4 sides
	g_pSurface->DrawFilledRect(x+r, y, x+w-r, y+r);
	g_pSurface->DrawFilledRect(x+w, y+r, x+w-r, y+h-r);
	g_pSurface->DrawFilledRect(x+w-r, y+h, x+r, y+h-r);
	g_pSurface->DrawFilledRect(x, y+h-r, x+r, y+r);
}

void DrawTexturedCenter(int x, int y, int w, int h, int r)
{
	//fill center
	g_pSurface->DrawTexturedRect(x+r, y+r, x+w-r, y+h-r);
}

void DrawFilledCenter(int x, int y, int w, int h, int r)
{
	//fill center
	g_pSurface->DrawFilledRect(x+r, y+r, x+w-r, y+h-r);
}

void DrawButton(int x, int y, int w, int h)
{
	g_pSurface->DrawSetTexture(g_texButtonEdge);
	DrawTexturedEdge(x, y, w, h, g_sizeButtonEdge);
	DrawFilledSide(x, y, w, h, g_sizeButtonEdge);
	DrawFilledCenter(x, y, w, h, g_sizeButtonEdge);
}

void DrawButtonEx(int x, int y, int w, int h, int edge)
{
	g_pSurface->DrawSetTexture(g_texButtonEdge);
	DrawTexturedEdge(x, y, w, h, edge);
	DrawFilledSide(x, y, w, h, edge);
	DrawFilledCenter(x, y, w, h, edge);
}

void DrawButtonLabel(wchar_t *label, int x, int y, int w, int h)
{
	int wt, ht;
	g_pSurface->GetTextSize(g_hButtonFont, label, wt, ht);
	g_pSurface->DrawSetTextFont(g_hButtonFont);
	g_pSurface->DrawSetTextPos(x+(w-wt)/2, y+(h-ht)/2);
	g_pSurface->DrawPrintText(label, wcslen(label));
	g_pSurface->DrawFlushText();
}

void DrawHudMenuKey(int x, int y, int w, int h)
{
	g_pSurface->DrawSetTexture(g_texHudMenuKey);
	g_pSurface->DrawTexturedRect(x, y, x+w, y+h);
}

void DrawHudMenuKeyLabel(wchar_t *label, int x, int y, int w, int h)
{
	int wt, ht;
	g_pSurface->GetTextSize(g_hHudMenuKeyFont, label, wt, ht);
	g_pSurface->DrawSetTextFont(g_hHudMenuKeyFont);
	g_pSurface->DrawSetTextPos(x+(w-wt)/2, y+(h-ht)/2);
	g_pSurface->DrawPrintText(label, wcslen(label));
	g_pSurface->DrawFlushText();
}

void DrawHudMenu(int x, int y, int w, int h)
{
	g_pSurface->DrawSetTexture(g_texHudMenuEdge);
	DrawTexturedEdge(x, y, w, h, g_sizeHudMenuEdge);

	g_pSurface->DrawSetTexture(g_texHudMenuSide);
	DrawTexturedSide(x, y, w, h, g_sizeHudMenuEdge);

	int r = g_sizeHudMenuEdge;
	qglBegin(GL_QUADS);
	qglTexCoord2f(0,0.33);
	qglVertex3f(x+r,y+r,0);
	qglTexCoord2f(1,0.33);
	qglVertex3f(x+w-r,y+r,0);
	qglTexCoord2f(1,1);
	qglVertex3f(x+w-r,y+h-r,0);
	qglTexCoord2f(0,1);
	qglVertex3f(x+r,y+h-r,0);
	qglEnd();
}

void DrawHudMask(int col, int x, int y, int w, int h)
{
	col = max(0, min(col, 2));
	if(col == 0 && (g_RefSupportExt & r_ext_shader))
	{
		gpRefExports->R_BeginDrawHudMask(41, 41, 41);

		g_pSurface->DrawSetTexture(g_texHudMaskEdge[0]);
		DrawTexturedEdge(x, y, w, h, g_sizeHudMaskEdge);

		g_pSurface->DrawSetTexture(g_texHudMaskSide[0]);
		DrawTexturedSide(x, y, w, h, g_sizeHudMaskEdge);

		int r = g_sizeHudMaskEdge;
		qglBegin(GL_QUADS);
		qglTexCoord2f(0,0.33);
		qglVertex3f(x+r,y+r,0);
		qglTexCoord2f(1,0.33);
		qglVertex3f(x+w-r,y+r,0);
		qglTexCoord2f(1,0.66);
		qglVertex3f(x+w-r,y+h-r,0);
		qglTexCoord2f(0,0.66);
		qglVertex3f(x+r,y+h-r,0);
		qglEnd();

		gpRefExports->ShaderAPI.GL_EndProgram();
	}
	else
	{
		g_pSurface->DrawSetTexture(g_texHudMaskEdge[col]);
		DrawTexturedEdge(x, y, w, h, g_sizeHudMaskEdge);

		g_pSurface->DrawSetTexture(g_texHudMaskSide[col]);
		DrawTexturedSide(x, y, w, h, g_sizeHudMaskEdge);

		int r = g_sizeHudMaskEdge;
		qglBegin(GL_QUADS);
		qglTexCoord2f(0,0.33);
		qglVertex3f(x+r,y+r,0);
		qglTexCoord2f(1,0.33);
		qglVertex3f(x+w-r,y+r,0);
		qglTexCoord2f(1,0.66);
		qglVertex3f(x+w-r,y+h-r,0);
		qglTexCoord2f(0,0.66);
		qglVertex3f(x+r,y+h-r,0);
		qglEnd();
	}
}

void R_DrawHitDamageText(const wchar_t *text, int width, int height)
{
	int w, h;
	g_pSurface->GetTextSize(g_hHitDamageFont, text, w, h);
	g_pSurface->DrawSetTextFont(g_hHitDamageFont);
	g_pSurface->DrawSetTextPos(width/2-w/2, height/2-h/2);
	g_pSurface->DrawPrintText(text, wcslen(text));
	g_pSurface->DrawFlushText();
}

void DrawTriangleFans(int centerX, int centerY, float radius, float start_radian, float end_radian, int step)
{
	if(step < 3 || end_radian < start_radian)
		return;

	float x, y, rad, radstep;

	radstep = (end_radian - start_radian) * (1.0f / step);
	rad = start_radian;

	qglBegin(GL_TRIANGLE_FAN);
	qglVertex3f(centerX, centerY, 0);
	for(int i = 0; i <= step; ++i)
	{
		x = centerX + radius * cos(rad);
		y = centerY + radius * sin(rad);
		qglVertex3f(x, y, 0);
		rad += radstep;
	}

	qglEnd();
}