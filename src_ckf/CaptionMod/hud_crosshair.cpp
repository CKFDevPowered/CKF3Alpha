#include "hud_base.h"
#include "client.h"
#include "cvar.h"

int g_texCrossHair[5];

int HudCrosshair_ChooseType(int WeaponId);

int HudCrosshair_VidInit(void)
{
	g_texCrossHair[0] = Surface_LoadTGA("resource\\tga\\crosshair1");
	g_texCrossHair[1] = Surface_LoadTGA("resource\\tga\\crosshair2");
	g_texCrossHair[2] = Surface_LoadTGA("resource\\tga\\crosshair3");
	g_texCrossHair[3] = Surface_LoadTGA("resource\\tga\\crosshair1");
	g_texCrossHair[4] = Surface_LoadTGA("resource\\tga\\crosshair2");

	return 1;
}

int HudCrosshair_Redraw(float flTime, int iIntermission)
{
	if(iIntermission)
		return 0;

	if((g_iHideHUD & HIDEHUD_WEAPONS))
		return 0;

	if(g_pBTEClient->GetHUDFov() < 60)
		return 0;

	if(g_crosshair_style->value <= 0)
		return 0;

	//calc RGB & alpha
	int r,g,b;
	sscanf(g_crosshair_color->string, "%d %d %d", &r, &g, &b);

	if(g_crosshair_style->value < 2) //g_crosshair_style 1 = TF2 style
	{
		int m_texchoose;
		m_texchoose = HudCrosshair_ChooseType(2);
		if (m_texchoose == 0) return 0; //If using PDA then return

		int x,y,w,h;
		g_pSurface->DrawGetTextureSize(m_texchoose, w, h);
		x = ScreenWidth/2;
		y = ScreenHeight/2;

		g_pSurface->DrawSetTexture(m_texchoose);
		g_pSurface->DrawSetColor(r,g,b,g_crosshair_a->value);
		g_pSurface->DrawGetTextureSize(m_texchoose, w, h);
		g_pSurface->DrawTexturedRect(x - w/2, y - h/2, x + w/2, y + h/2);
		return 0;
	}
	//g_crosshair_style 2 = cross with dot
	//g_crosshair_style 3 = cross only
	//g_crosshair_style 4 = dot only
	if(g_crosshair_style->value <5)
	{
		//	if (ChooseType(cl.pdata.weaponid)==0) return; //PDA

		//setup
		int x,y;
		float w,length,gap;
		w = g_crosshair_width->value;	
		length = g_crosshair_length->value;
		gap = g_crosshair_gap->value;
		x = ScreenWidth/2;
		y = ScreenHeight/2;

		//draw dot
		if(!(g_crosshair_style->value <4 && g_crosshair_style->value >2)){		//	if not 3 (cross only)
			g_pSurface->DrawSetTextColor(r,g,b,g_crosshair_a->value);
			g_pSurface->DrawFilledRect(x-w/2,y-w/2,x+w/2,y+w/2);
		}
		//draw cross
		if(!(g_crosshair_style->value <5 && g_crosshair_style->value >3))		//	if not 4 (dot only)
		{
			g_pSurface->DrawSetTextColor(r,g,b,g_crosshair_a->value);
			g_pSurface->DrawFilledRect(x-w/2,y-gap-length,w,length);		//draw up/north
			g_pSurface->DrawFilledRect(x-w/2,y+gap,w,length);		//draw down/south
			g_pSurface->DrawFilledRect(x-gap-length,y-w/2,length,w);		//draw left/west
			g_pSurface->DrawFilledRect(x+gap,y-w/2,length,w);		//draw right
		}
		return 0;
	}
	//If use "crosshair_custom.tga"
	if(g_crosshair_style->value < 6)
	{
		//		if (ChooseType(cl.pdata.weaponid)==0) return; //If using PDA then return
		int x,y,w,h;
		g_pSurface->DrawGetTextureSize(g_texCrossHair[4], w, h);
		x = ScreenWidth/2 - w/2;
		y = ScreenHeight/2 - h/2;
		g_pSurface->DrawSetTexture(g_texCrossHair[4]);
		g_pSurface->DrawSetColor(255, 255, 255, 255);
		g_pSurface->DrawGetTextureSize(g_texCrossHair[4], w, h);
		g_pSurface->DrawTexturedRect(x, y, x + w, y + h);
	}
	return 0;
}

int HudCrosshair_ChooseType(int WeaponId)
{
	int m_texchoose;
	switch(WeaponId)
	{
	case 2:
		m_texchoose = g_texCrossHair[0];
		break;
	case 3:
		m_texchoose = g_texCrossHair[2];
		break;
	case 5:
		m_texchoose = g_texCrossHair[2];
		break;
	case 10:
		m_texchoose = g_texCrossHair[2];
		break;
	case 11:
		m_texchoose = g_texCrossHair[2];
		break;
	case 13:
		m_texchoose = g_texCrossHair[1];
		break;
	case 15:
		m_texchoose = g_texCrossHair[0];
		break;
	case 17:
		m_texchoose = g_texCrossHair[0];
		break;
	case 20:
		m_texchoose = g_texCrossHair[2];
		break;
	case 23:
		m_texchoose = 0;
		break;
	case 25:
		m_texchoose = 0;
		break;
	case 26:
		m_texchoose = 0;
		break;
	default:
		m_texchoose = g_texCrossHair[3];
	}
	return m_texchoose;
}