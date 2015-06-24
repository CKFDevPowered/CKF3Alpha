#include "hud_base.h"
#include "exportfuncs.h"
#include "client.h"
#include "cvar.h"
#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"
#include "util.h"

static int g_texDisable;
static int g_texMenu;
static int g_texButton[4][3];

static vgui::HFont g_hPlayerNumFont;
static vgui::HFont g_hRandomFont;
static vgui::HFont g_hChooseTeamFont;
static vgui::HFont g_hSpectFont;

static xywh_t g_xywhMenu;
static xywh_t g_xywhButton[4];
static xywh_t g_xywhDisable[4];
static xywh_t g_xywhExit;

static xy_t g_xyRandomFont;
static xy_t g_xyChooseTeamFont;
static xy_t g_xyPlayerNumFont[2];
static xy_t g_xySpectFont;

static model_t *g_mdlTeamDoor;
static cl_entity_t g_DoorEntity[3];

static int g_iDoorState[3];
static float g_flDoorFrame[3];
static float g_flDoorAnim[3];

static int g_iSelectTeam;

static int g_iTeamIndex[] = {0, 3, 2, 1};
static char *g_szTeamNick[] = {"ran", "spe", "blu", "red"};

extern vec3_t g_vecZero;

static void UpdateModel(void)
{
	for(int i = 0; i < 3; ++i)
	{
		int state = 0;
		if(i == 0 && g_iSelectTeam == 1)
			state = 1;
		else if(i == 1 && g_iSelectTeam == 3)
			state = 1;
		else if(i == 2 && g_iSelectTeam == 4)
			state = 1;
		if(state)
		{
			if(g_iDoorState[i] == 0)
			{
				g_DoorEntity[i].curstate.sequence = 1;
				g_DoorEntity[i].curstate.frame = g_flDoorFrame[i];			
				g_DoorEntity[i].curstate.framerate = 1;
				g_DoorEntity[i].curstate.animtime = gEngfuncs.GetClientTime();
				g_flDoorAnim[i] = gEngfuncs.GetClientTime();
				g_iDoorState[i] = 1;
			}
		}
		else
		{
			if(g_iDoorState[i] == 1)
			{
				g_DoorEntity[i].curstate.sequence = 1;
				g_DoorEntity[i].curstate.frame = g_flDoorFrame[i];
				g_DoorEntity[i].curstate.framerate = -1;
				g_DoorEntity[i].curstate.animtime = gEngfuncs.GetClientTime();
				g_flDoorAnim[i] = gEngfuncs.GetClientTime();
				g_iDoorState[i] = 0;
			}
		}
		//calc frame
		if(g_iDoorState[i] == 0)
		{
			if(g_flDoorFrame[i] > 0)
			{
				g_flDoorFrame[i] = 255 - 255*(gEngfuncs.GetClientTime()-g_flDoorAnim[i])/0.4f;
				if(g_flDoorFrame[i] < 0) g_flDoorFrame[i] = 0;
			}
		}
		else
		{
			if(g_flDoorFrame[i] < 255)
			{
				g_flDoorFrame[i] = 255*(gEngfuncs.GetClientTime()-g_flDoorAnim[i])/0.4f;
				if(g_flDoorFrame[i] > 255) g_flDoorFrame[i] = 255;
			}
		}
	}
}

static void RenderModel(void)
{
	cl_entity_t *pEnt;
	cl_entity_t *pSaveEnt = *CurrentEntity;

	gpRefExports->R_PushRefDef();
	VectorClear(refdef->viewangles);
	VectorClear(refdef->vieworg);
	gpRefExports->R_UpdateRefDef();

	for(int i = 0; i < 3; ++i)
	{
		pEnt = &g_DoorEntity[i];

		VectorClear(pEnt->origin);
		VectorClear(pEnt->angles);

		pEnt->origin[0] = 49;
		pEnt->angles[1] = 0;

		*CurrentEntity = pEnt;

		int index = (i != 0) ? i+1 : i;
		int centerX = g_xywhButton[index].x+g_xywhButton[index].w/2;
		int centerY = g_xywhButton[index].y+g_xywhButton[index].h/2;
		if(g_3dmenu->value >= 2 && (g_RefSupportExt & r_ext_fbo))
		{
			int left = g_xywhButton[index].w/2;
			int top = g_xywhButton[index].h/2;

			gpRefExports->R_BeginDrawTrianglesInHUD_FBO(centerX, centerY, left, top);
			g_StudioRenderer.StudioDrawModel(STUDIO_RENDER);
			gpRefExports->R_FinishDrawTrianglesInHUD();

			if(g_RefSupportExt & r_ext_shader)
				gpRefExports->R_BeginFXAA(ScreenWidth, ScreenHeight);

			gpRefExports->R_Draw3DHUDQuad(centerX, centerY, left, top);

			if(g_RefSupportExt & r_ext_shader)
				gpRefExports->ShaderAPI.GL_EndProgram();
		}
		else
		{
			gpRefExports->R_BeginDrawTrianglesInHUD_Direct(centerX, centerY);
			g_StudioRenderer.StudioDrawModel(STUDIO_RENDER);
			gpRefExports->R_FinishDrawTrianglesInHUD();
		}
	}
	*CurrentEntity = pSaveEnt;
	gpRefExports->R_PopRefDef();
}

int HudTeamMenu_VidInit(void)
{
	g_texDisable = Surface_LoadTGA("resource\\tga\\ico_disable");
	g_texMenu = Surface_LoadTGA("resource\\tga\\ui_selteam");

	for(int i = 0; i < 4; ++i)
	{
		for(int j = 0; j < 3; ++j)
		{
			g_texButton[i][j] = Surface_LoadTGA(va("resource\\tga\\btn_sel%s_1440_%d", g_szTeamNick[i], j));
		}
	}
	g_texButton[1][2] = g_texButton[1][1];

	//calc position

	g_xyRandomFont.x = g_xOffset + ScreenHeight * .18;
	g_xyRandomFont.y = ScreenHeight * .12;
	g_xyChooseTeamFont.x = ScreenHeight * .05;
	g_xyChooseTeamFont.y = ScreenHeight * .92;
	g_xyPlayerNumFont[0].x = g_xOffset + ScreenHeight * .75;
	g_xyPlayerNumFont[0].y = ScreenHeight * .11;
	g_xyPlayerNumFont[1].x = g_xOffset + ScreenHeight * 1.14;
	g_xyPlayerNumFont[1].y = ScreenHeight * .11;
	g_xySpectFont.x = g_xOffset + ScreenHeight * .48;
	g_xySpectFont.y = ScreenHeight * .53;

	g_xywhMenu.x = g_bgOffset;
	g_xywhMenu.y = 0;
	g_xywhMenu.w = 16*ScreenHeight/9;
	g_xywhMenu.h = ScreenHeight;
	g_xywhExit.x = ScreenWidth - ScreenHeight * .4;
	g_xywhExit.y = ScreenHeight * .915;
	g_xywhExit.w = ScreenHeight * .3;
	g_xywhExit.h = ScreenHeight * .07;
	g_xywhButton[0].x = g_xOffset + ScreenHeight * .07;
	g_xywhButton[2].x = g_xOffset + ScreenHeight * .612;
	g_xywhButton[3].x = g_xOffset + ScreenHeight * .999;
	for(int i = 0; i < 4; ++i)
	{
		if(i == 1) continue;
		g_xywhButton[i].y = ScreenHeight * .215;
		g_xywhButton[i].w = ScreenHeight * .252;
		g_xywhButton[i].h = ScreenHeight * .645;
	}
	g_xywhButton[1].x = g_xOffset + ScreenHeight * .427;
	g_xywhButton[1].y = ScreenHeight * .505;
	g_xywhButton[1].w = ScreenHeight * .105;
	g_xywhButton[1].h = ScreenHeight * .085;

	g_xywhDisable[0].x = ScreenHeight * .05;
	g_xywhDisable[0].y = ScreenHeight * .2;
	g_xywhDisable[0].w = ScreenHeight * .16;
	g_xywhDisable[0].h = ScreenHeight * .16;
	g_xywhDisable[1].x = ScreenHeight * g_x->value;
	g_xywhDisable[1].y = ScreenHeight * g_y->value;
	g_xywhDisable[1].w = ScreenHeight * .12;
	g_xywhDisable[1].h = ScreenHeight * .12;
	//load model

	g_mdlTeamDoor = CL_LoadTentModel("models/CKF_III/ui_selteam_doors.mdl");

	for(int i = 0; i < 3; ++i)
	{
		CL_CreateTempEntity(&g_DoorEntity[i], g_mdlTeamDoor);
		g_DoorEntity[i].curstate.effects |= EF_3DMENU;
		g_DoorEntity[i].curstate.iuser1 = 128;
		g_DoorEntity[i].curstate.iuser2 = 192;
		g_DoorEntity[i].curstate.skin = i;
	}

	g_hPlayerNumFont = g_pSurface->CreateFont();
	g_hRandomFont = g_pSurface->CreateFont();
	g_hChooseTeamFont = g_pSurface->CreateFont();
	g_hSpectFont = g_pSurface->CreateFont();

	g_pSurface->AddGlyphSetToFont(g_hPlayerNumFont, "TF2 Build", 30 * ScreenHeight / 480, 0, 0, 0, vgui::ISurface::FONTFLAG_CUSTOM | vgui::ISurface::FONTFLAG_ANTIALIAS, 0x48, 0x57);
	g_pSurface->AddGlyphSetToFont(g_hRandomFont, "TF2", 17 * ScreenHeight / 480, 0, 0, 0, vgui::ISurface::FONTFLAG_CUSTOM | vgui::ISurface::FONTFLAG_ANTIALIAS, 0x0, 0xFFFF);
	g_pSurface->AddGlyphSetToFont(g_hChooseTeamFont, "TF2", 25 * ScreenHeight / 480, 0, 0, 0, vgui::ISurface::FONTFLAG_CUSTOM | vgui::ISurface::FONTFLAG_ANTIALIAS, 0x0, 0xFFFF);
	g_pSurface->AddGlyphSetToFont(g_hSpectFont, "TF2", max(13, 9 * ScreenHeight / 480), 0, 0, 0, vgui::ISurface::FONTFLAG_CUSTOM, 0x0, 0xFFFF);

	return 1;
}

void HudTeamMenu_Init(void)
{
	g_iDoorState[0] = 0;
	g_iDoorState[1] = 0;
	g_iDoorState[2] = 0;
	g_iSelectTeam = 0;
}

int CountTeamPlayers(int iTeam)
{
	int iCount = 0;
	for(int i=1; i<=gEngfuncs.GetMaxClients(); i++)
	{
		if(!g_HudPlayerInfo[i].name || !g_HudPlayerInfo[i].name[0])
			continue;
		if((g_PlayerInfo[i].iTeam == iTeam) || (iTeam == 3 && g_PlayerInfo[i].iTeam == 0))
			iCount++;
	}
	return iCount;
}

int HudTeamMenu_Redraw(float flTime, int iIntermission)
{
	wchar_t szText[8];

	if(g_iMenu != MENU_CHOOSETEAM)
		return 0;

	int iState;
	int mx, my;
	gEngfuncs.GetMousePosition(&mx, &my);

	g_xywhDisable[0].x = ScreenHeight * g_x->value;
	g_xywhDisable[0].y = ScreenHeight * g_y->value;
	g_xywhDisable[0].w = ScreenHeight * g_w->value;
	g_xywhDisable[0].h = ScreenHeight * g_h->value;

	g_iSelectTeam = 0;

	g_pSurface->DrawSetColor(255,255,255,255);
	g_pSurface->DrawSetTexture(g_texMenu);
	g_pSurface->DrawTexturedRect(g_xywhMenu.x, g_xywhMenu.y, g_xywhMenu.x+g_xywhMenu.w, g_xywhMenu.y+g_xywhMenu.h);

	//menu buttons
	g_pSurface->DrawSetColor(255,255,255,255);

	//random button
	for(int i = 0; i < 4; ++i)
	{
		iState = 0;
		if(!g_iSelectTeam && (g_iMenuKeys & (1<<i)) && HudBase_IsMouseInRect(mx, my, 
			g_xywhButton[i].x,
			g_xywhButton[i].y,
			g_xywhButton[i].w,
			g_xywhButton[i].h))
		{
			iState = (g_mouse_state == 1) ? 2 : 1;
			g_iSelectTeam = i+1;
		}
		//don't draw door texture when 3dmenu is on
		if(i != 1 && g_3dmenu->value)
			continue;
		g_pSurface->DrawSetTexture(g_texButton[i][iState]);
		g_pSurface->DrawTexturedRect(
			g_xywhButton[i].x, 
			g_xywhButton[i].y, 
			g_xywhButton[i].x+g_xywhButton[i].w, 
			g_xywhButton[i].y+g_xywhButton[i].h);
		if(!(g_iMenuKeys & (1<<i)))
		{
			g_pSurface->DrawSetTexture(g_texDisable);
			g_pSurface->DrawTexturedRect(
				g_xywhButton[i].x+g_xywhDisable[i].x,
				g_xywhButton[i].y+g_xywhDisable[i].y,
				g_xywhButton[i].x+g_xywhDisable[i].x+g_xywhDisable[i].w,
				g_xywhButton[i].y+g_xywhDisable[i].y+g_xywhDisable[i].h);
		}
	}

	g_pSurface->DrawSetTextColor(42,39,37,255);
	g_pSurface->DrawSetTextFont(g_hRandomFont);
	int tw, th;
	g_pSurface->GetTextSize(g_hRandomFont, g_wszRandom, tw, th);
	g_pSurface->DrawSetTextPos(g_xyRandomFont.x - tw/2, g_xyRandomFont.y);
	g_pSurface->DrawPrintText(g_wszRandom, wcslen(g_wszRandom));

	g_pSurface->DrawSetTextColor(201,188,163,255);
	g_pSurface->DrawSetTextFont(g_hChooseTeamFont);
	g_pSurface->DrawSetTextPos(g_xyChooseTeamFont.x, g_xyChooseTeamFont.y);
	g_pSurface->DrawPrintText(g_wszChooseTeam, wcslen(g_wszChooseTeam));

	HudBase_GetPlayerInfo();

	wsprintfW(szText, L"%d", CountTeamPlayers(2));
	g_pSurface->DrawSetTextColor(42,39,37,255);
	g_pSurface->DrawSetTextFont(g_hPlayerNumFont);
	g_pSurface->DrawSetTextPos(g_xyPlayerNumFont[0].x, g_xyPlayerNumFont[0].y);
	g_pSurface->DrawPrintText(szText, wcslen(szText));

	wsprintfW(szText, L"%d", CountTeamPlayers(1));
	g_pSurface->DrawSetTextColor(42,39,37,255);
	g_pSurface->DrawSetTextFont(g_hPlayerNumFont);
	g_pSurface->DrawSetTextPos(g_xyPlayerNumFont[1].x, g_xyPlayerNumFont[1].y);
	g_pSurface->DrawPrintText(szText, wcslen(szText));

	int r, g, b;
	int r2, g2, b2;
	if(g_iMenuKeys & MENUKEY_0)
	{
		if(HudBase_IsMouseInRect(mx, my, g_xywhExit.x, g_xywhExit.y, g_xywhExit.w, g_xywhExit.h))
		{
			r = 146; g = 71; b = 56;
			r2 = 233; g2 = 222; b2 = 200;
		}
		else
		{
			r = 118; g = 107; b = 97;
			r2 = 69; g2 = 63; b2 = 57;
		}
		g_pSurface->DrawSetColor(r, g, b, 255);
		DrawButton(g_xywhExit.x, g_xywhExit.y, g_xywhExit.w, g_xywhExit.h);
		g_pSurface->DrawSetTextColor(r2, g2, b2, 255);
		DrawButtonLabel(g_wszExit, g_xywhExit.x, g_xywhExit.y, g_xywhExit.w, g_xywhExit.h);
	}

	g_pSurface->DrawSetTextColor(255,255,255,255);
	g_pSurface->DrawSetTextFont(g_hSpectFont);
	g_pSurface->GetTextSize(g_hSpectFont, g_wszTeamMenu_Spect, tw, th);
	g_pSurface->DrawSetTextPos(g_xySpectFont.x - tw / 2, g_xySpectFont.y);
	g_pSurface->DrawPrintText(g_wszTeamMenu_Spect, wcslen(g_wszTeamMenu_Spect));

	g_pSurface->DrawFlushText();

	if(g_3dmenu->value != 0)
	{
		UpdateModel();
		RenderModel();
	}

	return 1;
}

void HudTeamMenu_MouseUp(int mx, int my)
{
	if(g_iMenu != MENU_CHOOSETEAM)
		return;

	char cmd[32];

	//random button
	if(g_iSelectTeam >= 1 && g_iSelectTeam <= 4)
	{
		g_iMenu = 0;
		HudBase_ActivateMouse();
		sprintf(cmd, "jointeam %d\n", g_iTeamIndex[g_iSelectTeam-1]);
		gEngfuncs.pfnClientCmd(cmd);
		return;
	}
	if(HudBase_IsMouseInRect(mx, my, g_xywhExit.x, g_xywhExit.y, g_xywhExit.w, g_xywhExit.h) && (g_iMenuKeys & MENUKEY_0))
	{
		g_iMenu = 0;
		HudBase_ActivateMouse();
		gEngfuncs.pfnClientCmd("closemenu\n");
		return;
	}
}

int HudTeamMenu_KeyEvent(int eventcode, int keynum, const char *pszCurrentBinding)
{
	if(g_iMenu != MENU_CHOOSETEAM)
		return 0;

	char cmd[32];
	int i;

	if('1' <= keynum && keynum <= '4')
	{
		if(eventcode != 0)
			return 1;

		i = keynum - '1';
		if(g_iMenuKeys & (1<<i))
		{	
			g_iMenu = 0;
			HudBase_ActivateMouse();

			sprintf(cmd, "jointeam %d\n", g_iTeamIndex[i]);
			gEngfuncs.pfnClientCmd(cmd);
		}
		return 1;
	}
	if('X' == keynum && (g_iMenuKeys & MENUKEY_0))
	{
		if(eventcode != 0)
			return 1;

		g_iMenu = 0;
		HudBase_ActivateMouse();
		gEngfuncs.pfnClientCmd("closemenu\n");
		return 1;
	}
	return 0;
}