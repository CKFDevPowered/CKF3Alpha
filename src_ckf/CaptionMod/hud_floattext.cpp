#include "hud_base.h"
#include "exportfuncs.h"
#include "client.h"
#include "cvar.h"

static vgui::HFont g_hAddHealthFont;

class CHudFloatText
{
public:
	void Draw(void)
	{
		int w, h;
		float frac = (m_die - g_flClientTime) / m_life;
		float frac2 = 1 - frac;
		g_pSurface->DrawSetTextColor(m_col.r, m_col.g, m_col.b, m_col.a * min(frac * 2, 1));
		g_pSurface->GetTextSize(m_font, m_text, w, h);
		g_pSurface->DrawSetTextPos(m_x + frac2 * m_xoffset, m_y + frac2 * m_yoffset);
		g_pSurface->DrawSetTextFont(m_font);
		g_pSurface->DrawPrintText(m_text, wcslen(m_text));
		g_pSurface->DrawFlushText();
	}
public:
	vgui::HFont m_font;
	wchar_t m_text[32];
	colorVec m_col;
	int m_x, m_y;
	float m_xoffset, m_yoffset;
	float m_die;
	float m_life;
};

std::vector<CHudFloatText> g_FloatTexts;

int HudFloatText_VidInit(void)
{
	g_hAddHealthFont = g_pSurface->CreateFont();
	g_pSurface->AddGlyphSetToFont(g_hAddHealthFont, "TF2", 24 * ScreenHeight / 480, 0, 0, 0, vgui::ISurface::FONTFLAG_CUSTOM | vgui::ISurface::FONTFLAG_ANTIALIAS, 0x48, 0x57);

	return 1;
}

void HudFloatText_AddHealth(int iHealth)
{
	if(iHealth == 0)
		return;
	CHudFloatText ft;

	ft.m_font = g_hAddHealthFont;
	ft.m_x = 75*(ScreenHeight*4/3)/640;
	ft.m_y = (480 - 87)*ScreenHeight/480;

	ft.m_xoffset = 0;
	ft.m_yoffset = -ScreenHeight * 0.1;

	if(iHealth > 0)
	{
		ft.m_col.r = 0;
		ft.m_col.g = 255;
		ft.m_col.b = 0;
		ft.m_col.a = 180;
		wsprintfW(ft.m_text, L"+%d", iHealth);
	}
	else
	{
		ft.m_col.r = 255;
		ft.m_col.g = 0;
		ft.m_col.b = 0;
		ft.m_col.a = 180;
		wsprintfW(ft.m_text, L"-%d", iHealth);
	}
	ft.m_life = 1.5;
	ft.m_die = g_flClientTime + ft.m_life;

	g_FloatTexts.push_back(ft);
}

void HudFloatText_AddMetal(int iMetal)
{
	if(iMetal == 0)
		return;

	CHudFloatText ft;

	ft.m_font = g_hAddHealthFont;

	ft.m_x=(640 - 111)*(ScreenHeight*4/3)/640+ScreenWidth-ScreenHeight*4/3;
	ft.m_y=(480 - 34 )*ScreenHeight/480;

	ft.m_xoffset = 0;
	ft.m_yoffset = -ScreenHeight * 0.1;

	if(iMetal > 0)
	{
		ft.m_col.r = 0;
		ft.m_col.g = 255;
		ft.m_col.b = 0;
		ft.m_col.a = 180;
		wsprintfW(ft.m_text, L"+%d", iMetal);
	}
	else
	{
		ft.m_col.r = 255;
		ft.m_col.g = 0;
		ft.m_col.b = 0;
		ft.m_col.a = 180;
		wsprintfW(ft.m_text, L"-%d", iMetal);
	}
	ft.m_life = 1.5;
	ft.m_die = g_flClientTime + ft.m_life;

	g_FloatTexts.push_back(ft);
}

int HudFloatText_Redraw(float flTime, int iIntermission)
{
	if(iIntermission)
		return 0;
	
	std::vector<CHudFloatText>::iterator it = g_FloatTexts.begin();

	while(it != g_FloatTexts.end())
	{
		if(flTime > it->m_die)
		{
			it = g_FloatTexts.erase(it);
			continue;
		}

		it->Draw();
		
		it++;
	}
	return 1;
}