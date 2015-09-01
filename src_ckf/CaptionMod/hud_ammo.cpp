#include "hud_base.h"
#include "client.h"
#include "cvar.h"
#include "qgl.h"
#include "weapon.h"
#include "exportfuncs.h"

#define HUD_AMMO_NORMAL 0
#define	HUD_AMMO_CLIPONLY 1
#define	HUD_AMMO_MELEE 2
#define	HUD_AMMO_MEDIGUN 3

static int g_texAmmoMask[2];
static int g_texAmmoMaskUber[2];
static int g_texAmmoSmall[2];
static int g_texAmmoStick[2];
static int g_texAmmoMetal;
static int g_texIcoStickbomb[2];

static vgui::HFont g_hUberEnergyFont;
static vgui::HFont g_hClipFont;
static vgui::HFont g_hAmmoFont;
static vgui::HFont g_hStickFont;
static vgui::HFont g_hMetalFont;
static vgui::HFont g_hSniperChargeFont;

static xywh_t g_xywhSniperCharge;
static xy_t g_xySniperChargeFont;
static int g_texScopeNum;

static int g_texSniperDot[2];

void HudAmmo_DrawMask(void);
void DrawSpy(void);
void DrawMetal(void);
void DrawStickBomb(void);
void DrawSniperCharge(void);

int Choosetype(int weaponid);

extern float g_flTraceDistance;

int HudAmmo_VidInit(void)
{
	g_texAmmoMask[0] = Surface_LoadTGA("resource\\tga\\mask_weapon_red");
	g_texAmmoMask[1] = Surface_LoadTGA("resource\\tga\\mask_weapon_blu");
	g_texAmmoMaskUber[0] = Surface_LoadTGA("resource\\tga\\mask_uber_red");
	g_texAmmoMaskUber[1] = Surface_LoadTGA("resource\\tga\\mask_uber_blu");
	g_texAmmoSmall[0] = Surface_LoadTGA("resource\\tga\\mask_weapon_small_red");
	g_texAmmoSmall[1] = Surface_LoadTGA("resource\\tga\\mask_weapon_small_blu");
	g_texAmmoStick[0] = Surface_LoadTGA("resource\\tga\\mask_demo_red");
	g_texAmmoStick[1] = Surface_LoadTGA("resource\\tga\\mask_demo_blu");
	g_texAmmoMetal = Surface_LoadTGA("resource\\tga\\mask_weapon_metal");
	g_texIcoStickbomb[0] = Surface_LoadTGA("resource\\tga\\ico_stickybomb_red");
	g_texIcoStickbomb[1] = Surface_LoadTGA("resource\\tga\\ico_stickybomb_blu");

	g_hUberEnergyFont = g_pSurface->CreateFont();
	g_hAmmoFont = g_pSurface->CreateFont();
	g_hClipFont = g_pSurface->CreateFont();
	g_hStickFont = g_pSurface->CreateFont();
	g_hMetalFont = g_pSurface->CreateFont();
	g_hUberEnergyFont = g_pSurface->CreateFont();
	g_hSniperChargeFont = g_hUberEnergyFont;

	g_pSurface->AddGlyphSetToFont(g_hUberEnergyFont, "TF2", 11 * ScreenHeight / 480, 0, 0, 0, vgui::ISurface::FONTFLAG_CUSTOM | vgui::ISurface::FONTFLAG_ANTIALIAS, 0x48, 0x57);
	g_pSurface->AddGlyphSetToFont(g_hClipFont, "TF2 Build", 46 * ScreenHeight / 480, 0, 0, 0, vgui::ISurface::FONTFLAG_CUSTOM | vgui::ISurface::FONTFLAG_ANTIALIAS, 0x48, 0x57);
	g_pSurface->AddGlyphSetToFont(g_hAmmoFont, "TF2", 24 * ScreenHeight / 480, 0, 0, 0, vgui::ISurface::FONTFLAG_CUSTOM | vgui::ISurface::FONTFLAG_ANTIALIAS, 0x48, 0x57);
	g_pSurface->AddGlyphSetToFont(g_hStickFont, "TF2", 30 * ScreenHeight / 480, 0, 0, 0, vgui::ISurface::FONTFLAG_CUSTOM | vgui::ISurface::FONTFLAG_ANTIALIAS, 0x48, 0x57);
	g_pSurface->AddGlyphSetToFont(g_hMetalFont, "TF2", 20 * ScreenHeight / 480, 0, 0, 0, vgui::ISurface::FONTFLAG_CUSTOM | vgui::ISurface::FONTFLAG_ANTIALIAS, 0x48, 0x57);

	//scope charge stuff
	g_texScopeNum = Surface_LoadTGA("resource\\tga\\scope_number");
	g_xywhSniperCharge.x = g_xOffset + ScreenHeight  * .75;
	g_xywhSniperCharge.y = ScreenHeight * .55;
	g_xywhSniperCharge.w = ScreenHeight * .2;
	g_xywhSniperCharge.h = g_xywhSniperCharge.w * 42 / 86;
	g_xySniperChargeFont.x = ScreenHeight * .18;
	g_xySniperChargeFont.y = ScreenHeight * .02;

	g_texSniperDot[0] = Surface_LoadTGA("resource\\tga\\sniperdot_red");
	g_texSniperDot[1] = Surface_LoadTGA("resource\\tga\\sniperdot_blue");
	return 1;
}

int HudAmmo_Redraw(float flTime, int iIntermission)
{
	if(iIntermission)
		return 0;

	if((g_iHideHUD & HIDEHUD_WEAPONS))
		return 0;

	if(gEngfuncs.IsSpectateOnly())
		return 0;

	//draw clip,ammo,basic mask and medic's uber
	HudAmmo_DrawMask();

	DrawStickBomb();
	DrawSniperCharge();

	if(g_iClass == CLASS_ENGINEER)
		DrawMetal();

	else if(g_iClass == CLASS_SPY)
		DrawSpy();

	return 1;
}

void HudAmmo_DrawMask(void)
{
	wchar_t wszClip[6];										//for number text draw
	wchar_t wszAmmo[6];
	wchar_t wszUber[32];
	int x,y,w,h,type;
	float yscale,xscale;

	if(!g_Player.m_pActiveItem)
		return;

	int iAmmo = g_Player.m_pActiveItem->m_iAmmo;
	int iClip = g_Player.m_pActiveItem->m_iClip;
	type = Choosetype(g_Player.m_pActiveItem->m_iId);

	if (type==HUD_AMMO_NORMAL || type==HUD_AMMO_CLIPONLY)		// draw weapon mask
	{
		x=(640 - 89.6f)*(ScreenHeight*4/3)/640+ScreenWidth-ScreenHeight*4/3;		// 89.6 decide x to right screen border, no need to change the rest
		y=(480 - 55.47f)*ScreenHeight/480;
		g_pSurface->DrawGetTextureSize(g_texAmmoMask[g_iTeam-1], w, h);
		yscale = (ScreenHeight*45.86f/h)/480;		
		g_pSurface->DrawSetTexture(g_texAmmoMask[g_iTeam-1]);
		g_pSurface->DrawSetColor(255,255,255,255);
		g_pSurface->DrawTexturedRect(x,y,x+w*yscale, y+h*yscale);
	}
	else if (type==HUD_AMMO_MEDIGUN)								 // draw uber mask
	{
		x=(640 - 135)*(ScreenHeight*4/3)/640+ScreenWidth-ScreenHeight*4/3;
		y=(480 - 63.47)*ScreenHeight/480;
		g_pSurface->DrawGetTextureSize(g_texAmmoMaskUber[g_iTeam-1], w, h);
		xscale = (ScreenHeight*55.47f/h)/480;
		g_pSurface->DrawSetTexture(g_texAmmoMaskUber[g_iTeam-1]);
		g_pSurface->DrawSetColor(255,255,255,255);
		g_pSurface->DrawTexturedRect(x,y,x+w*xscale, y+h*xscale);

		wsprintfW(wszUber, L"UBERCHARGE: %d%%", (int)g_Player.m_fUbercharge);
		int cx,cy;
		cx=(640 - 108)*(ScreenHeight*4/3)/640+ScreenWidth-ScreenHeight*4/3;
		cy=(480 - 42)*ScreenHeight/480;

		g_pSurface->DrawSetTextColor(255, 255, 255, 200);
		g_pSurface->DrawSetTextPos(cx, cy);
		g_pSurface->DrawSetTextFont(g_hUberEnergyFont);
		g_pSurface->DrawPrintText(wszUber, wcslen(wszUber));
		g_pSurface->DrawFlushText();

		//uber BG
		x=(640 - 108)*(ScreenHeight*4/3)/640+ScreenWidth-ScreenHeight*4/3;
		y=(480 - 32)*ScreenHeight/480;
		w = (ScreenHeight*85.0f)/480;
		h = (ScreenHeight*8.0f)/480;

		qglEnable(GL_BLEND);
		g_pSurface->DrawSetColor(255, 255, 255, 60);
		g_pSurface->DrawFilledRect(x,y,x+w,y+h);
		//uber front
		float ubercal = g_Player.m_fUbercharge*85.0f/100;
		w = (ScreenHeight*ubercal)/480;

		if (g_Player.m_fUbercharge < 1)
			return;

		else if (g_Player.m_fUbercharge >= 1 && g_Player.m_fUbercharge <100)
		{
			g_pSurface->DrawSetColor(255, 255, 255, 255);
			g_pSurface->DrawFilledRect(x,y,x+w,y+h);
		}
		else if (g_Player.m_fUbercharge >=100)
		{
			float flTime = g_flClientTime * 2;
			float flDelta = flTime - (float)((int)flTime);
			int u = abs(flDelta - 0.5f) * 2 * 127 + 128;
			g_pSurface->DrawSetColor(u, u, u, 255);
			g_pSurface->DrawFilledRect(x,y,x+w,y+h);
		}
	}
	if (type==HUD_AMMO_NORMAL)		//draw text, Clip+Ammo style
	{
		//calc xy
		int cx,cy,ax,ay;
		cx=(640 - 48)*(ScreenHeight*4/3)/640+ScreenWidth-ScreenHeight*4/3;
		cy=(480 - 65)*ScreenHeight/480;
		ax=(640 - 48)*(ScreenHeight*4/3)/640+ScreenWidth-ScreenHeight*4/3;
		ay=(480 - 41)*ScreenHeight/480;

		wsprintfW(wszAmmo, L"%d", iAmmo);
		wsprintfW(wszClip, L"%d", iClip);

		g_pSurface->GetTextSize(g_hClipFont, wszClip, w, h);		//clip needs right-align

		g_pSurface->DrawSetTextColor(50, 50, 50, 255);
		g_pSurface->DrawSetTextPos(cx-w, cy);
		g_pSurface->DrawSetTextFont(g_hClipFont);
		g_pSurface->DrawPrintText(wszClip, wcslen(wszClip));
	
		g_pSurface->DrawSetTextColor(255, 255, 255, 255);
		g_pSurface->DrawSetTextPos(cx-w-2, cy-2);
		g_pSurface->DrawPrintText(wszClip, wcslen(wszClip));
		g_pSurface->DrawFlushText();

		//draw ammo numbers
		g_pSurface->DrawSetTextColor(50, 50, 50, 255);
		g_pSurface->DrawSetTextPos(ax, ay);
		g_pSurface->DrawSetTextFont(g_hAmmoFont);
		g_pSurface->DrawPrintText(wszAmmo, wcslen(wszAmmo));

		g_pSurface->DrawSetTextColor(255, 255, 255, 255);
		g_pSurface->DrawSetTextPos(ax-2, ay-2);
		g_pSurface->DrawPrintText(wszAmmo, wcslen(wszAmmo));
		g_pSurface->DrawFlushText();
	}
	else if (type==HUD_AMMO_CLIPONLY)		//draw text, Clip
	{
		//calc xy
		int cx,cy;
		cx=(640 - 18)*(ScreenHeight*4/3)/640+ScreenWidth-ScreenHeight*4/3;
		cy=(480 - 55)*ScreenHeight/480;
		wsprintfW(wszClip, L"%d", iAmmo);

		g_pSurface->GetTextSize(g_hClipFont, wszClip, w, h);

		g_pSurface->DrawSetTextColor(50, 50, 50, 255);
		g_pSurface->DrawSetTextPos(cx-w, cy);
		g_pSurface->DrawSetTextFont(g_hClipFont);
		g_pSurface->DrawPrintText(wszClip, wcslen(wszClip));

		g_pSurface->DrawSetTextColor(255, 255, 255, 255);
		g_pSurface->DrawSetTextPos(cx-w-2, cy-2);
		g_pSurface->DrawPrintText(wszClip, wcslen(wszClip));
		g_pSurface->DrawFlushText();
	}
}

extern CClientSniperifle g_Sniperifle;

void DrawSniperCharge(void)
{
	if(!g_Player.m_pActiveItem)
		return;

	if(g_Player.m_pActiveItem->m_iId != WEAPON_SNIPERIFLE)
		return;

	if(!(g_Sniperifle.m_iWeaponState & WEAPONSTATE_CHARGING))
		return;

	int w, h;
	wchar_t szText[8];

	float charge = g_Sniperifle.m_fCharge / 100.0;

	qglTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	qglEnable(GL_TEXTURE_2D);
	qglEnable(GL_BLEND);
	qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	qglBindTexture(GL_TEXTURE_2D, g_texScopeNum);
	qglColor4ub(255,255,255,255);

	qglBegin(GL_QUADS);
	qglTexCoord2f(0,0);
	qglVertex3f(g_xywhSniperCharge.x,g_xywhSniperCharge.y,0);
	qglTexCoord2f(charge*.75f,0);
	qglVertex3f(g_xywhSniperCharge.x+g_xywhSniperCharge.w*charge*.75f,g_xywhSniperCharge.y,0);
	qglTexCoord2f(charge*.75f,1);
	qglVertex3f(g_xywhSniperCharge.x+g_xywhSniperCharge.w*charge*.75f,g_xywhSniperCharge.y+g_xywhSniperCharge.h,0);
	qglTexCoord2f(0,1);
	qglVertex3f(g_xywhSniperCharge.x,g_xywhSniperCharge.y+g_xywhSniperCharge.h,0);
	qglEnd();

	if(g_Sniperifle.m_fSpotActive)
	{
		g_pSurface->DrawSetTexture(g_texSniperDot[g_iTeam-1]);
		g_pSurface->DrawSetColor(255,255,255,255);

		float flScale;
		if(g_flTraceDistance <= g_sniperdot_mindist->value)
			flScale = g_sniperdot_max->value;
		else if(g_flTraceDistance < g_sniperdot_maxdist->value)
			flScale = g_sniperdot_min->value + (g_sniperdot_maxdist->value-g_flTraceDistance) * (g_sniperdot_max->value-g_sniperdot_min->value) / (g_sniperdot_maxdist->value-g_sniperdot_mindist->value);
		else
			flScale = g_sniperdot_min->value;

		w = .05f * ScreenHeight * flScale;
		h = w;

		qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		g_pSurface->DrawTexturedRect(ScreenWidth/2-w, ScreenHeight/2-h, ScreenWidth/2+w, ScreenHeight/2+h);

		w *= charge;
		h = w;

		qglEnable(GL_BLEND);
		qglBlendFunc(GL_SRC_ALPHA, GL_ONE);
		qglBegin(GL_QUADS);
		qglTexCoord2f(0,0);
		qglVertex3f(ScreenWidth/2-w,ScreenHeight/2-h,0);
		qglTexCoord2f(1,0);
		qglVertex3f(ScreenWidth/2+w,ScreenHeight/2-h,0);
		qglTexCoord2f(1,1);
		qglVertex3f(ScreenWidth/2+w,ScreenHeight/2+h,0);
		qglTexCoord2f(0,1);
		qglVertex3f(ScreenWidth/2-w,ScreenHeight/2+h,0);
		qglEnd();

		qglBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	wsprintfW(szText, L"%d%%", (int)g_Sniperifle.m_fCharge);
	g_pSurface->DrawSetTextColor(244, 192, 80, 255);
	g_pSurface->GetTextSize(g_hSniperChargeFont, szText, w, h);
	g_pSurface->DrawSetTextPos(g_xywhSniperCharge.x+g_xySniperChargeFont.x-w, g_xywhSniperCharge.y+g_xySniperChargeFont.y);
	g_pSurface->DrawSetTextFont(g_hSniperChargeFont);
	g_pSurface->DrawPrintText(szText, wcslen(szText));	
	g_pSurface->DrawFlushText();
}

extern CClientStickyLauncher g_StickyLauncher;

void DrawStickBomb(void)
{
	wchar_t wszArmor[6];
	int x,y,w,h,w2,h2,w3,h3;

	if(!g_pClientWeapon[WEAPON_SLOT_SECONDARY])
		return;

	if(g_pClientWeapon[WEAPON_SLOT_SECONDARY]->m_iId != WEAPON_STICKYLAUNCHER)
		return;

	x=ScreenWidth-ScreenHeight * .35;
	y=ScreenHeight * .89;
	g_pSurface->DrawGetTextureSize(g_texAmmoStick[g_iTeam-1], w, h);

	w2 = ScreenHeight * .16;
	h2 = ScreenHeight * .16 * h / w;
	g_pSurface->DrawSetTexture(g_texAmmoStick[g_iTeam-1]);
	g_pSurface->DrawSetColor(255, 255, 255, 255);
	g_pSurface->DrawTexturedRect(x, y, x+w2, y+h2);
	
	int bx,by;
	bx=ScreenWidth-ScreenHeight * .31;//
	by=ScreenHeight * .915;//
	w3 = ScreenHeight * .045;//
	h3 = w3;
	g_pSurface->DrawSetTexture(g_texIcoStickbomb[g_iTeam-1]);
	g_pSurface->DrawSetColor(255, 255, 255, 255);
	g_pSurface->DrawTexturedRect(bx, by, bx+w3, by+h3);

	int ax,ay;
	ax=x+0.11 * ScreenHeight;
	ay=y+0.02 * ScreenHeight;

	wsprintfW(wszArmor, L"%d", g_StickyLauncher.m_iStickyNum);

	g_pSurface->GetTextSize(g_hStickFont, wszArmor, w, h);

	g_pSurface->DrawSetTextColor(50, 50, 50, 255);
	g_pSurface->DrawSetTextPos((ax-w/2)+2, ay+2);
	g_pSurface->DrawSetTextFont(g_hStickFont);
	g_pSurface->DrawPrintText(wszArmor, wcslen(wszArmor));

	g_pSurface->DrawSetTextColor(255, 255, 255, 255);
	g_pSurface->DrawSetTextPos(ax-w/2, ay);
	g_pSurface->DrawSetTextFont(g_hStickFont);
	g_pSurface->DrawPrintText(wszArmor, wcslen(wszArmor));
	g_pSurface->DrawFlushText();
}

void DrawMetal(void)
{
	wchar_t wszArmor[6];
	int x,y,w,h;
	float scale;
	x=(640 - 149)*(ScreenHeight*4/3)/640+ScreenWidth-ScreenHeight*4/3;
	y=(480 - 46)*ScreenHeight/480;
	g_pSurface->DrawGetTextureSize(g_texAmmoSmall[g_iTeam-1], w, h);
	scale = (ScreenHeight*40.0f/h)/480;

	g_pSurface->DrawSetTexture(g_texAmmoSmall[g_iTeam-1]);
	g_pSurface->DrawSetColor(255,255,255,255);
	g_pSurface->DrawTexturedRect(x,y,x+w*scale,y+h*scale);
	//Metal tga
	x=(640 - 143)*(ScreenHeight*4/3)/640+ScreenWidth-ScreenHeight*4/3;
	y=(480 - 36)*ScreenHeight/480;
	g_pSurface->DrawGetTextureSize(g_texAmmoMetal, w, h);
	scale = (ScreenHeight*15.0f/h)/480;

	g_pSurface->DrawSetTexture(g_texAmmoMetal);
	g_pSurface->DrawSetColor(255,255,255,255);
	g_pSurface->DrawTexturedRect(x,y,x+w*scale,y+h*scale);

	int ax,ay;
	ax=(640 - 111)*(ScreenHeight*4/3)/640+ScreenWidth-ScreenHeight*4/3;
	ay=(480 - 34 )*ScreenHeight/480;

	wsprintfW(wszArmor, L"%d", g_Player.m_iMetal);
	g_pSurface->GetTextSize(g_hMetalFont, wszArmor, w, h);

	g_pSurface->DrawSetTextColor(50, 50, 50, 255);
	g_pSurface->DrawSetTextPos(ax-w/2, ay);
	g_pSurface->DrawSetTextFont(g_hMetalFont);
	g_pSurface->DrawPrintText(wszArmor, wcslen(wszArmor));
	g_pSurface->DrawSetTextColor(255, 255, 255, 255);
	g_pSurface->DrawSetTextPos(ax-w/2 -2, ay -2);
	g_pSurface->DrawSetTextFont(g_hMetalFont);
	g_pSurface->DrawPrintText(wszArmor, wcslen(wszArmor));

	g_pSurface->DrawFlushText();
}

void DrawSpy(void)
{
	wchar_t wszArmor[16];
	int x,y,w,h;
	float scale;
	x=(640 - 149)*(ScreenHeight*4/3)/640+ScreenWidth-ScreenHeight*4/3;
	y=(480 - 46)*ScreenHeight/480;
	g_pSurface->DrawGetTextureSize(g_texAmmoSmall[g_iTeam-1], w, h);
	scale = (ScreenHeight*38.0f/h)/480;

	g_pSurface->DrawSetTexture(g_texAmmoSmall[g_iTeam-1]);
	g_pSurface->DrawSetColor(255,255,255,255);
	g_pSurface->DrawTexturedRect(x,y,x+w*scale,y+h*scale);
	int ax,ay;
	ax=(640 - 138)*(ScreenHeight*4/3)/640+ScreenWidth-ScreenHeight*4/3;
	ay=(480 - 34)*ScreenHeight/480;

	wsprintfW(wszArmor, L"%s: %d%%", g_wszCloakEnergy, (int)g_Player.m_flCloakEnergy);
	g_pSurface->DrawSetTextColor(255, 255, 255, 255);
	g_pSurface->DrawSetTextPos(ax, ay);
	g_pSurface->DrawSetTextFont(g_hUberEnergyFont);
	g_pSurface->DrawPrintText(wszArmor, wcslen(wszArmor));
	g_pSurface->DrawFlushText();

	float xscale,yscale;
	x=(640 - 140)*(ScreenHeight*4/3)/640+ScreenWidth-ScreenHeight*4/3;
	y=(480 - 20)*ScreenHeight/480;
	xscale = (ScreenHeight*42)/480;
	yscale = (ScreenHeight*4)/480;
	qglEnable(GL_BLEND);
	g_pSurface->DrawSetColor(255, 255, 255, 60);
	g_pSurface->DrawFilledRect(x,y,x+xscale,y+yscale);
	if (g_Player.m_flCloakEnergy > 0)
	{
		xscale = (g_Player.m_flCloakEnergy * ScreenHeight*42)/48000;
		g_pSurface->DrawSetColor(255, 255, 255, 255);
		g_pSurface->DrawFilledRect(x,y,x+xscale,y+yscale);
	}
}

int Choosetype(int weaponid)
{
	int m_typechoose;
	switch(weaponid)
	{
	case WEAPON_SNIPERIFLE: case WEAPON_MINIGUN: case WEAPON_FLAMETHROWER: 
		m_typechoose = HUD_AMMO_CLIPONLY;
		break;
	case WEAPON_KUKRI: case WEAPON_SHOVEL: case WEAPON_BOTTLE: case WEAPON_BAT: case WEAPON_FIST: case WEAPON_FIREAXE: case WEAPON_BUTTERFLY: case WEAPON_WRENCH:
		m_typechoose = HUD_AMMO_MELEE;
		break;
	case WEAPON_MEDIGUN: case WEAPON_BONESAW:
		m_typechoose = HUD_AMMO_MEDIGUN;
		break;
	case WEAPON_SAPPER:
	case WEAPON_DISGUISEKIT:
	case WEAPON_BUILDPDA:
	case WEAPON_DESTROYPDA:
		m_typechoose = HUD_AMMO_MELEE;
		break;
	default:
		m_typechoose = HUD_AMMO_NORMAL;
		break;
	}
	return m_typechoose;
}