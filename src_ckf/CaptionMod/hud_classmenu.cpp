#include "hud_base.h"
#include "exportfuncs.h"
#include "client.h"
#include "cvar.h"
#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"
#include "util.h"
#include "studio_util.h"
#include "cJSON.h"

#include <string>
#include <vector>

int g_texMenu;
int g_texButton[10][3];
int g_texFontIcon;

static vgui::HFont g_hChooseClassFont;
static vgui::HFont g_hClassNumFont;
static vgui::HFont g_hClassifyFont;
static vgui::HFont g_hIntroFont;

static int g_sClassIndex[] = {1, 3, 4, 8, 2, 7, 6, 5, 9, 10};
static int g_iSelectClass;
static int g_iLastSelectClass;

typedef struct
{
	std::vector<std::wstring> text[10];
	std::string buf;
}classintro_t;

static classintro_t g_ClassIntro;

static char *g_szClassNick[] = {"sco", "sol", "pyr", "dem", "hvy", "eng", "med", "sni", "spy", "ran"};
static char *g_szClassName[] = {"scout", "soldier", "pyro", "demoman", "heavy", "engineer", "medic", "sniper", "spy", "random"};
static int g_iWeaponBody[] = {9, 6, 66, 0, 3, 3, 27, 57, 48};

static xywh_t g_xywhMenu;
static xywh_t g_xywhButton[10];
static xywh_t g_xywhExit;
static xy_t g_xyChooseClassFont;
static xy_t g_xyClassNumFont[9];
static xy_t g_xyClassifyFont[3];
static xy_t g_xyIntroFont;

extern model_t *g_mdlPlayer[10];

static cl_entity_t g_PlayerEntity;

static void UpdateModel(void)
{
	if(g_iSelectClass >= 1 && g_iSelectClass <= 10)
	{
		if(g_iLastSelectClass != g_iSelectClass)
		{
			int i = g_iSelectClass - 1;
			g_PlayerEntity.model = g_mdlPlayer[g_sClassIndex[i]-1];
			g_PlayerEntity.index = gEngfuncs.GetLocalPlayer()->index;
			g_PlayerEntity.curstate.skin = (g_iTeam == 1) ? 0 : 1;

			if(g_sClassIndex[i] == CLASS_HEAVY || g_sClassIndex[i] == CLASS_DEMOMAN)
				g_PlayerEntity.curstate.weaponmodel = gEngfuncs.pEventAPI->EV_FindModelIndex("models/CKF_III/wp_group_2bone.mdl");
			else
				g_PlayerEntity.curstate.weaponmodel = gEngfuncs.pEventAPI->EV_FindModelIndex("models/CKF_III/wp_group_rf.mdl");
			g_PlayerEntity.curstate.scale = g_iWeaponBody[i];
			g_PlayerEntity.curstate.sequence = (g_sClassIndex[i] == CLASS_MEDIC) ? 19 : 45;
		}
		g_iLastSelectClass = g_iSelectClass;
	}
}

static void RenderModel(void)
{
	if(g_PlayerEntity.model == NULL)
		return;
	
	cl_entity_t *pEnt = &g_PlayerEntity;

	gpRefExports->R_PushRefDef();
	VectorClear(refdef->viewangles);
	VectorClear(refdef->vieworg);
	gpRefExports->R_UpdateRefDef();
	VectorClear(pEnt->origin);
	VectorClear(pEnt->angles);

	pEnt->origin[0] = 100;
	pEnt->angles[1] = 180;

	pEnt->curstate.colormap = (g_iTeam == 1) ? (1 | (1<<8)) : (140 | (140<<8));
	
	cl_entity_t *pSaveEnt = *CurrentEntity;
	*CurrentEntity = pEnt;

	int centerX = ScreenWidth * 0.28;
	int centerY = ScreenHeight * 0.62;
	if(g_3dmenu->value >= 2 && (g_RefSupportExt & r_ext_fbo))
	{
		int left = ScreenHeight * 0.3;
		int top = ScreenHeight * 0.4;

		gpRefExports->R_BeginDrawTrianglesInHUD_FBO(centerX, centerY, left, top);
		g_StudioRenderer.StudioDrawPlayer_3DHUD();
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
		g_StudioRenderer.StudioDrawPlayer_3DHUD();
		gpRefExports->R_FinishDrawTrianglesInHUD();
	}

	*CurrentEntity = pSaveEnt;
	gpRefExports->R_PopRefDef();
}

void ClassIntro_ParseBuf(void)
{
	cJSON *root = cJSON_Parse(g_ClassIntro.buf.c_str());
	if (!root)
		return;
	for(int i = 0; i < 10; ++i)
	{
		cJSON *classobj = cJSON_GetObjectItem(root, g_szClassName[i]);
		if(!classobj)
			continue;
		int num = cJSON_GetArraySize(classobj);
		for(int j = 0; j < num; j++)
		{
			cJSON *strobj = cJSON_GetArrayItem(root, j);
			if(!strobj || !strobj->valuestring)
				continue;
			g_ClassIntro.text[i].push_back(UTF8ToUnicode(strobj->valuestring));
		}
	}
	cJSON_Delete(root);
	g_ClassIntro.buf.clear();
}

void ClassIntro_ClearBuf(void)
{
	g_ClassIntro.buf.clear();
}

void ClassIntro_AddToBuf(const char *str)
{
	g_ClassIntro.buf.append(str);
}

int ClassIntro_VidInit(void)
{
	for(int i = 0; i < 10; ++i)
		g_ClassIntro.text[i].clear();

	for(int i = 0; i < 10; ++i)
	{
		for(int j = 0; j < 4; ++j)
		{
			if(wcslen(g_wszClassTips[i][j]) > 0)
			{
				g_ClassIntro.text[i].push_back(std::wstring(g_wszClassTips[i][j]));
			}
		}
	}

	return 1;
}

int HudClassMenu_VidInit(void)
{
	g_texMenu = Surface_LoadTGA("resource\\tga\\ui_selclass");
	g_texFontIcon = Surface_LoadTGA("resource\\tga\\chalf_circle");

	char tempName[64];
	for(int i = 0; i < 10; ++i)
	{
		for(int j = 0; j < 3; ++j)
		{
			sprintf(tempName, "gfx/vgui/class/class_sel_sm_%s_%d", g_szClassName[i], j ? "blu" : "red" );
			g_texButton[i][j] = Surface_LoadTGA(tempName);
		}
	}

	g_xywhMenu.x = g_bgOffset;
	g_xywhMenu.w = 16*ScreenHeight/9;
	g_xywhMenu.y = 0;
	g_xywhMenu.h = ScreenHeight;
	g_xyChooseClassFont.x = ScreenHeight * .05;
	g_xyChooseClassFont.y = ScreenHeight * .92;

	for(int i = 0; i < 3; ++i)
	{
		for(int j = 0; j < 3; ++j)
		{
			g_xyClassNumFont[j+i*3].x = g_xOffset + ScreenHeight * (.058+.39*i+.1*j);
			g_xyClassNumFont[j+i*3].y = ScreenHeight * 0.167;
			g_xywhButton[j+i*3].x = g_xOffset + ScreenHeight * (.05+.39*i+.1*j);
			g_xywhButton[j+i*3].w = ScreenHeight * .1;
			g_xywhButton[j+i*3].y = ScreenHeight * 0;
			g_xywhButton[j+i*3].h = ScreenHeight * .2;
		}
		g_xyClassifyFont[i].x = g_xOffset + ScreenHeight * (.042+.39*i);
		g_xyClassifyFont[i].y = ScreenHeight * .2;
	}
	g_xywhButton[9].x = g_xOffset + ScreenHeight * 1.19;
	g_xywhButton[9].w = ScreenHeight * .1;
	g_xywhButton[9].y = ScreenHeight * 0;
	g_xywhButton[9].h = ScreenHeight * .2;
	g_xywhExit.x = ScreenWidth - ScreenHeight * .4;
	g_xywhExit.y = ScreenHeight * .915;
	g_xywhExit.w = ScreenHeight * .3;
	g_xywhExit.h = ScreenHeight * .07;
	g_xyIntroFont.x = g_xOffset + ScreenHeight * .75;
	g_xyIntroFont.y = ScreenHeight * .35;

	//Create FakePlayer TEnt
	CL_CreateTempEntity(&g_PlayerEntity, NULL);
	g_PlayerEntity.index = 0;
	g_PlayerEntity.player = 1;
	g_PlayerEntity.curstate.effects |= EF_3DMENU;
	g_PlayerEntity.curstate.iuser1 = 0;
	g_PlayerEntity.curstate.iuser2 = 192;
	g_PlayerEntity.curstate.vuser1[0] = 0;
	g_PlayerEntity.curstate.vuser1[1] = 0;
	g_PlayerEntity.curstate.gaitsequence = 1;

	g_hChooseClassFont = g_pSurface->CreateFont();
	g_hClassNumFont = g_pSurface->CreateFont();
	g_hClassifyFont = g_pSurface->CreateFont();
	g_hIntroFont = g_pSurface->CreateFont();

	g_pSurface->AddGlyphSetToFont(g_hChooseClassFont, "TF2", 25 * ScreenHeight / 480, 0, 0, 0, vgui::ISurface::FONTFLAG_CUSTOM | vgui::ISurface::FONTFLAG_ANTIALIAS, 0x0, 0xFFFF);
	g_pSurface->AddGlyphSetToFont(g_hClassNumFont, "TF2", 14 * ScreenHeight / 480, 0, 0, 0, vgui::ISurface::FONTFLAG_CUSTOM | vgui::ISurface::FONTFLAG_ANTIALIAS, 0x0, 0xFFFF);
	g_pSurface->AddGlyphSetToFont(g_hClassifyFont, "TF2", max(13 * ScreenHeight / 480, 14), 0, 0, 0, vgui::ISurface::FONTFLAG_CUSTOM | vgui::ISurface::FONTFLAG_ANTIALIAS, 0x0, 0xFFFF);
	g_pSurface->AddGlyphSetToFont(g_hIntroFont, "TF2 Secondary", 14 * ScreenHeight / 480, 0, 0, 0, vgui::ISurface::FONTFLAG_CUSTOM | vgui::ISurface::FONTFLAG_ANTIALIAS, 0x0, 0xFFFF);

	ClassIntro_VidInit();

	return 1;
}

void HudClassMenu_Init(void)
{
	g_iSelectClass = 0;
	g_iLastSelectClass = 0;
}

int HudClassMenu_Redraw(float flTime, int iIntermission)
{
	wchar_t szText[32];

	if(g_iMenu != MENU_CHOOSECLASS)
		return 0;

	g_pSurface->DrawSetColor(255,255,255,255);
	g_pSurface->DrawSetTexture(g_texMenu);
	g_pSurface->DrawTexturedRect(g_xywhMenu.x, g_xywhMenu.y, g_xywhMenu.x+g_xywhMenu.w, g_xywhMenu.y+g_xywhMenu.h);

	g_pSurface->DrawSetTextColor(201,188,163,255);
	g_pSurface->DrawSetTextFont(g_hChooseClassFont);
	g_pSurface->DrawSetTextPos(g_xyChooseClassFont.x, g_xyChooseClassFont.y);
	g_pSurface->DrawPrintText(g_wszChooseClass, wcslen(g_wszChooseClass));

	if(g_iSelectClass >= 1 && g_iSelectClass <= 10)
	{
		int i = g_iSelectClass-1;
		int introY = g_xyIntroFont.y;
		int ln;
		int size = g_ClassIntro.text[i].size();
		for(int j = 0; j < size; ++j)
		{
			std::wstring &str = g_ClassIntro.text[i][j];
			g_pSurface->DrawSetTexture(g_texFontIcon);
			g_pSurface->DrawSetColor(201,188,163,255);
			g_pSurface->DrawTexturedRect(g_xyIntroFont.x, introY, g_xyIntroFont.x+12 * ScreenHeight / 480, introY+12 * ScreenHeight / 480);
			
			g_pSurface->DrawSetTextColor(233,233,233,255);
			g_pSurface->DrawSetTextFont(g_hIntroFont);
			HudBase_DrawMultilineSetup(2, 180 * ScreenHeight / 480);
			ln = HudBase_DrawMultiline(str.c_str(), g_xyIntroFont.x + 14 * ScreenHeight / 480, introY);
			introY += ln * 18 * ScreenHeight / 480;
		}
	}

	g_pSurface->DrawFlushText();

	g_pSurface->DrawSetColor(255,255,255,255);

	int iState;
	int mx, my;
	gEngfuncs.GetMousePosition(&mx, &my);

	g_iSelectClass = 0;

	for(int i = 0; i < 10; ++i)
	{
		iState = 0;
		if(HudBase_IsMouseInRect(mx, my, 
			g_xywhButton[i].x+4*ScreenHeight/480, 
			g_xywhButton[i].y+24*ScreenHeight/480, 
			g_xywhButton[i].w-8*ScreenHeight/480, 
			g_xywhButton[i].h-24*ScreenHeight/480
			))
		{
			iState = (g_iTeam == 1) ? 1 : 2;
			g_iSelectClass = i+1;
		}
		g_pSurface->DrawSetTexture(g_texButton[i][iState]);
		g_pSurface->DrawTexturedRect(g_xywhButton[i].x, g_xywhButton[i].y, g_xywhButton[i].x+g_xywhButton[i].w, g_xywhButton[i].y+g_xywhButton[i].h);
	}

	for(int i = 0; i < 9; ++i)
	{
		wsprintfW(szText, L"%d", i+1);
		g_pSurface->DrawSetTextColor(119,110,102,255);
		g_pSurface->DrawSetTextFont(g_hClassNumFont);
		g_pSurface->DrawSetTextPos(g_xyClassNumFont[i].x, g_xyClassNumFont[i].y);
		g_pSurface->DrawPrintText(szText, wcslen(szText));
		if(i % 3 == 0)
		{
			g_pSurface->DrawSetTextFont(g_hClassifyFont);
			g_pSurface->DrawSetTextPos(g_xyClassifyFont[i/3].x, g_xyClassifyFont[i/3].y);
			g_pSurface->DrawPrintText(g_wszClassify[i/3], wcslen(g_wszClassify[i/3]));			
		}
	}

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

	g_pSurface->DrawFlushText();

	if(g_3dmenu->value > 0)
	{
		UpdateModel();
		RenderModel();
	}

	return 1;
}

void HudClassMenu_MouseUp(int mx, int my)
{
	if(g_iMenu != MENU_CHOOSECLASS)
		return;

	char cmd[32];

	//random button
	if(g_iSelectClass >= 1 && g_iSelectClass <= 10)
	{
		if(g_sClassIndex[g_iSelectClass-1] != g_iClass)
		{
			g_iMenu = 0;
			sprintf(cmd, "joinclass %d\n", g_sClassIndex[g_iSelectClass-1]);
			gEngfuncs.pfnClientCmd(cmd);
			HudBase_ActivateMouse();
		}
	}
	if(HudBase_IsMouseInRect(mx, my, g_xywhExit.x, g_xywhExit.y, g_xywhExit.w, g_xywhExit.h) && (g_iMenuKeys & MENUKEY_0))
	{
		g_iMenu = 0;
		HudBase_ActivateMouse();
		gEngfuncs.pfnClientCmd("closemenu\n");
		return;
	}
}

int HudClassMenu_KeyEvent(int eventcode, int keynum, const char *pszCurrentBinding)
{
	if(g_iMenu != MENU_CHOOSECLASS)
		return 0;

	char cmd[32];

	if('1' <= keynum && keynum <= '9')
	{
		if(eventcode != 0)
			return 1;
		int i = keynum-'1';
		if(g_iClass != g_sClassIndex[i])
		{
			g_iMenu = 0;
			sprintf(cmd, "joinclass %d\n", g_sClassIndex[i]);
			gEngfuncs.pfnClientCmd(cmd);
			HudBase_ActivateMouse();
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