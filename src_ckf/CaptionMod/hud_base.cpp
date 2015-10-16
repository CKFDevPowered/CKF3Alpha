#include "hud_base.h"
#include "client.h"
#include "Encode.h"

SCREENINFO g_ScreenInfo;

int g_mouse_state;
int g_mouse_oldstate;

bool g_bGameUIActivate;

typedef struct
{
	int yOffset;
	int limitWidth;
}draw_multiline_t;

static draw_multiline_t g_DrawMultiLine;

extern HFont g_hCurrentFont;

int HudHealth_VidInit(void);
int HudHealth_Redraw(float flTime, int iIntermission);

int HudClass_VidInit(void);
int HudClass_Redraw(float flTime, int iIntermission);

int HudAmmo_VidInit(void);
int HudAmmo_Redraw(float flTime, int iIntermission);

int HudCrosshair_VidInit(void);
int HudCrosshair_Redraw(float flTime, int iIntermission);

int HudTeamMenu_VidInit(void);
int HudTeamMenu_Redraw(float flTime, int iIntermission);
void HudTeamMenu_MouseUp(int mx, int my);
int HudTeamMenu_KeyEvent(int eventcode, int keynum, const char *pszCurrentBinding);
void HudTeamMenu_Init(void);

int HudClassMenu_VidInit(void);
int HudClassMenu_Redraw(float flTime, int iIntermission);
void HudClassMenu_MouseUp(int mx, int my);
int HudClassMenu_KeyEvent(int eventcode, int keynum, const char *pszCurrentBinding);
void HudClassMenu_Init(void);

int HudIntroMenu_Redraw(float flTime, int iIntermission);
void HudIntroMenu_MouseUp(int mx, int my);
int HudIntroMenu_KeyEvent(int eventcode, int keynum, const char *pszCurrentBinding);
int HudIntroMenu_VidInit(void);
void HudIntroMenu_Init(void);

int HudCommon_VidInit(void);
int HudOverlay_VidInit(void);
int HudOverlay_Redraw(float flTime, int iIntermission);

int HudBuildMenu_VidInit(void);
int HudBuildMenu_Redraw(float flTime, int iIntermission);
int HudBuildMenu_KeyEvent(int eventcode, int keynum, const char *pszCurrentBinding);
int HudDemolishMenu_VidInit(void);
int HudDemolishMenu_Redraw(float flTime, int iIntermission);
int HudDemolishMenu_KeyEvent(int eventcode, int keynum, const char *pszCurrentBinding);

int HudDisguiseMenu_VidInit(void);
int HudDisguiseMenu_Redraw(float flTime, int iIntermission);
int HudDisguiseMenu_KeyEvent(int eventcode, int keynum, const char *pszCurrentBinding);

int HudStatusBar_VidInit(void);
int HudStatusBar_Redraw(float flTime, int iIntermission);

int HudBuild_VidInit(void);
int HudBuild_Redraw(float flTime, int iIntermission);
void HudBuild_Init(void);

int HudDeathMsg_Redraw(float flTime, int iIntermission);
int HudDeathMsg_VidInit(void);
void HudDeathMsg_Init(void);

int HudFloatText_VidInit(void);
int HudFloatText_Redraw(float flTime, int iIntermission);
//
//void HudScoreBoard_Init(void);
//int HudScoreBoard_VidInit(void);
//int HudScoreBoard_Redraw(float flTime, int iIntermission);

void HudObject_Init(void);
int HudObject_VidInit(void);
int HudObject_Redraw(float flTime, int iIntermission);

int HudLocalize_VidInit(void);

HCURSOR g_hCursorArrow;

int g_bgOffset;
int g_xOffset;

extern qboolean g_iHudVidInitalized;
extern int g_RefSupportExt;

//加载固定资源在这里。刚进游戏加载完client.dll时调用
void HudBase_Init(void)
{
	g_ScreenInfo.iSize = sizeof(g_ScreenInfo);
	gEngfuncs.pfnGetScreenInfo(&g_ScreenInfo);

	g_mouse_state = g_mouse_oldstate = 0;

	g_bgOffset = (ScreenWidth - 16*ScreenHeight/9)/2;
	g_xOffset = (ScreenWidth - 4*ScreenHeight/3)/2;

	g_bGameUIActivate = true;
	g_iHudVidInitalized = false;
	g_hCursorArrow = LoadCursor(NULL, IDC_ARROW);
	g_RefSupportExt = gRefExports.R_GetSupportExtension();
}

//加载地图相关资源在这里
int HudBase_VidInit(void)
{
	if(!g_iHudVidInitalized)
	{
		HudCommon_VidInit();
		HudLocalize_VidInit();

		HudHealth_VidInit();
		HudClass_VidInit();
		HudAmmo_VidInit();
		HudCrosshair_VidInit();
		//HudOverlay_VidInit();

		//HudIntroMenu_VidInit();
		//HudTeamMenu_VidInit();
		//HudClassMenu_VidInit();
		HudBuildMenu_VidInit();
		HudDemolishMenu_VidInit();
		HudDisguiseMenu_VidInit();
		HudStatusBar_VidInit();
		HudBuild_VidInit();

		//HudDeathMsg_VidInit();
		HudFloatText_VidInit();
		//HudScoreBoard_VidInit();
		HudObject_VidInit();
	}

	//HudIntroMenu_Init();
	//HudTeamMenu_Init();
	//HudClassMenu_Init();
	HudBuild_Init();
	//HudDeathMsg_Init();
	//HudScoreBoard_Init();
	HudObject_Init();

	return 1;
}

int HudBase_Redraw(float flTime, int iIntermission)
{
	//HudOverlay_Redraw(flTime, iIntermission);
	//HudDeathMsg_Redraw(flTime, iIntermission);
	HudObject_Redraw(flTime, iIntermission);
	if(!(g_iHideHUD & HIDEHUD_ALL))
	{
		HudBuild_Redraw(flTime, iIntermission);
		HudAmmo_Redraw(flTime, iIntermission);
	}

	HudFloatText_Redraw(flTime, iIntermission);
	HudStatusBar_Redraw(flTime, iIntermission);

	if(g_iHudMenu)
	{
		HudBuildMenu_Redraw(flTime, iIntermission);
		HudDemolishMenu_Redraw(flTime, iIntermission);
		HudDisguiseMenu_Redraw(flTime, iIntermission);
	}
	HudCrosshair_Redraw(flTime, iIntermission);

	return 1;
}

void HudBase_MouseUp(int mx, int my)
{
	//if(g_bGameUIActivate)
	//	return;

	//HudIntroMenu_MouseUp(mx, my);
	//HudTeamMenu_MouseUp(mx, my);
	//HudClassMenu_MouseUp(mx, my);	
}

void HudBase_MouseDown(int mx, int my)
{
	//if(g_bGameUIActivate)
	//	return;
}

int HudBase_KeyEvent(int eventcode, int keynum, const char *pszCurrentBinding)
{
	//if(g_bGameUIActivate)
	//	return 0;

	//if(HudBase_IsFullScreenMenu())
	//{
	//	if(HudIntroMenu_KeyEvent(eventcode, keynum, pszCurrentBinding))
	//		return 1;

	//	if(HudTeamMenu_KeyEvent(eventcode, keynum, pszCurrentBinding))
	//		return 1;

	//	if(HudClassMenu_KeyEvent(eventcode, keynum, pszCurrentBinding))
	//		return 1;
	//}
	//else
	//{
		if(HudBuildMenu_KeyEvent(eventcode, keynum, pszCurrentBinding))
			return 1;
		if(HudDemolishMenu_KeyEvent(eventcode, keynum, pszCurrentBinding))
			return 1;
		if(HudDisguiseMenu_KeyEvent(eventcode, keynum, pszCurrentBinding))
			return 1;
	//}
	return 0;
}

//void HudBase_DrawMultilineSetup(int yOffset, int limitWidth)
//{
//	g_DrawMultiLine.yOffset = yOffset;
//	g_DrawMultiLine.limitWidth = limitWidth;
//}
//
//int HudBase_DrawMultiline(const wchar_t *text, int x, int y)
//{
//	static wchar_t buf[1024];
//	wchar_t *p;
//	int w, h, ln;
//	int limitWidth = g_DrawMultiLine.limitWidth;
//	p = (wchar_t *)text;
//	ln = 0;
//	while(*p)
//	{
//		int i = 0;
//		int m = 0;
//		while(1)
//		{
//			buf[i++] = *p++;
//			buf[i] = 0;
//			g_pSurface->GetTextSize(g_hCurrentFont, buf, w, h);
//			if(*p == L'\0' || *p == L'\n' || w > limitWidth)
//			{
//				buf[i] = 0;
//				if(*p == L'\0')
//					break;
//				if(*p != L'\n' && w >= limitWidth)
//					m ++;
//				while(*p == L'\n')
//				{
//					m ++;
//					p ++;
//				}
//				break;
//			}
//		}
//		g_pSurface->DrawSetTextPos(x, y);
//		g_pSurface->DrawPrintText(buf, wcslen(buf));
//		m = max(m, 1);
//		y += (h + g_DrawMultiLine.yOffset)*m;
//		ln += m;
//	}
//	return ln;
//}

int Surface_LoadTGA(const char *filename)
{
	int texID = g_pSurface->CreateNewTextureID();
	g_pSurface->DrawSetTextureFile(texID, filename, 1, 0);
	return texID;
}

bool HudBase_IsMouseInRect(int mx, int my, int x, int y, int w, int h)
{
	//if(g_bGameUIActivate)
	//	return false;
	if( mx > x && mx < x+w && my > y && my < y+h)
		return true;
	return false;
}

void HudBase_GetPlayerInfo(void)
{
	for ( int i = 1; i < 33; i++ )
	{
		gEngfuncs.pfnGetPlayerInfo( i, &g_HudPlayerInfo[i] );
	}
}

bool HudBase_IsFullScreenMenu(void)
{
	//if(g_iMenu == MENU_CHOOSETEAM || g_iMenu == MENU_CHOOSECLASS || g_iMenu == MENU_INTRO)
	//	return true;

	return false;
}

void HudBase_DeactivateMouse(void)
{
	g_pSurface->SetCursor(vgui::dc_arrow);
	SetCursor(g_hCursorArrow);

	//gExportfuncs.IN_DeactivateMouse();
	//g_iForceFOV = 90;
}

void HudBase_ActivateMouse(void)
{
	g_pSurface->SetCursor(vgui::dc_none);
	SetCursor(NULL);

	//gExportfuncs.IN_ActivateMouse();
	//g_iForceFOV = 0;
}

void LocalizeStringUTF8(char *inputString, int inputStringLength, const char *tokenString)
{
	wchar_t *pString = g_pVGuiLocalize->Find(tokenString);
	if(pString)
	{
		if(inputString && inputStringLength)
		{
			strncpy(inputString, UnicodeToUTF8(pString), inputStringLength-1);
			inputString[inputStringLength-1] = '\0';
		}
	}
	else
	{
		if(inputString && inputStringLength)
		{
			strncpy(inputString, tokenString, inputStringLength-1);
			inputString[inputStringLength-1] = '\0';
		}
	}
}

void LocalizeStringUnicode(wchar_t *inputStringW, int inputStringWLength, const char *tokenString)
{
	wchar_t *pString = g_pVGuiLocalize->Find(tokenString);
	if(pString)
	{
		if(inputStringW && inputStringWLength)
		{
			wcsncpy(inputStringW, pString, inputStringWLength-1);
			inputStringW[inputStringWLength-1] = L'\0';
		}
	}
	else
	{
		if(inputStringW && inputStringWLength)
		{
			wcsncpy(inputStringW, ANSIToUnicode(tokenString), inputStringWLength-1);
			inputStringW[inputStringWLength-1] = L'\0';
		}
	}
}