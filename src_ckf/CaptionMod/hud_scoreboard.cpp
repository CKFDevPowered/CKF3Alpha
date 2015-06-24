#include "hud_base.h"
#include "exportfuncs.h"
#include "client.h"
#include "cvar.h"
#include "util.h"

int g_ScoreBoardEnabled;

//team board
static xywh_t g_xywhBlueBoard;
static xy_t g_xyBlueTeam;
static xy_t g_xyBluePlayerNum;
static xy_t g_xyBlueTeamScore;

static xywh_t g_xywhRedBoard;
static xy_t g_xyRedTeam;
static xy_t g_xyRedPlayerNum;
static xy_t g_xyRedTeamScore;

//main board
static xywh_t g_xywhMainBoard;
static xy_t g_xyBluePlayerInfo;
static xy_t g_xyRedPlayerInfo;
static xy_t g_xyServerName;
static xy_t g_xyMapName;

static xy_t g_xyTitleName[2];
static xy_t g_xyTitleScore[2];
static xy_t g_xyTitleDeath[2];
static xy_t g_xyTitlePing[2];

//bottom board
static xywh_t g_xywhBottomBoard;
static xywh_t g_xywhBottomClassIcon;
static xy_t g_xyBottomPlayerName;
static xy_t g_xySpectators;

static xy_t g_xyPlayerStats[STATS_MAX_NUM-1];

//fonts

static vgui::HFont g_hFontTeamName;
static vgui::HFont g_hFontPlayerNum;
static vgui::HFont g_hFontScore;
static vgui::HFont g_hFontTeamScore;
static vgui::HFont g_hFontBottomPlayerName;

static xywh_t g_xyBoardLine[4];

static color24 g_ubColor[] = {{235,225,200}, {160,50,50}, {154,205,255}};

void DrawHudMask(int col, int x, int y, int w, int h);
extern int g_texPlayerMan[2][9];

static int g_iRedPlayerIndexs[32];
static int g_iBluePlayerIndexs[32];
static int g_iSpectatorIndexs[32];

int HudScoreBoard_VidInit(void)
{
	//team board blue
	g_xywhBlueBoard.x = g_xOffset + ScreenHeight * (6 / 600.0);
	g_xywhBlueBoard.y = ScreenHeight * (44 / 600.0);
	g_xywhBlueBoard.w = ScreenHeight * (390 / 600.0);
	g_xywhBlueBoard.h = ScreenHeight * (49 / 600.0);

	g_xyBlueTeam.x = g_xywhBlueBoard.x + ScreenHeight * (9 / 600.0);
	g_xyBlueTeam.y = g_xywhBlueBoard.y + ScreenHeight * (10 / 600.0);

	g_xyBluePlayerNum.x = g_xywhBlueBoard.x + ScreenHeight * (158 / 600.0);
	g_xyBluePlayerNum.y = g_xywhBlueBoard.y + ScreenHeight * (24 / 600.0);

	g_xyBlueTeamScore.x = g_xOffset + ScreenHeight * (342 / 600.0);
	g_xyBlueTeamScore.y = ScreenHeight * (30 / 600.0);

	//team board red
	g_xywhRedBoard.x = g_xOffset + ScreenHeight * (402 / 600.0);
	g_xywhRedBoard.y = g_xywhBlueBoard.y;
	g_xywhRedBoard.w = g_xywhBlueBoard.w;
	g_xywhRedBoard.h = g_xywhBlueBoard.h;

	g_xyRedTeam.x = g_xywhRedBoard.x + ScreenHeight * (317 / 600.0);
	g_xyRedTeam.y = g_xyBlueTeam.y;

	g_xyRedPlayerNum.x = g_xywhRedBoard.x + ScreenHeight * (144 / 600.0);
	g_xyRedPlayerNum.y = g_xyBluePlayerNum.y;

	g_xyRedTeamScore.x = g_xOffset + ScreenHeight * (412 / 600.0);
	g_xyRedTeamScore.y = g_xyBlueTeamScore.y;

	//main board
	g_xywhMainBoard.x = g_xOffset + ScreenHeight * (4 / 600.0);
	g_xywhMainBoard.y = ScreenHeight * (99 / 600.0);
	g_xywhMainBoard.w = ScreenHeight * (793 / 600.0);
	g_xywhMainBoard.h = ScreenHeight * (478 / 600.0);

	g_xyBluePlayerInfo.x = g_xywhMainBoard.x + ScreenHeight * (7 / 600.0);
	g_xyBluePlayerInfo.y = g_xywhMainBoard.y + ScreenHeight * (36 / 600.0);

	g_xyRedPlayerInfo.x = g_xywhMainBoard.x + ScreenHeight * (407 / 600.0);
	g_xyRedPlayerInfo.y = g_xyBluePlayerInfo.y;

	g_xyServerName.x = g_xywhMainBoard.x + ScreenHeight * (63 / 600.0);
	g_xyServerName.y = g_xywhMainBoard.y + ScreenHeight * (4 / 600.0);

	g_xyMapName.x = g_xywhMainBoard.x + g_xywhMainBoard.w - ScreenHeight * (41 / 600.0);
	g_xyMapName.y = g_xywhMainBoard.y + ScreenHeight * (4 / 600.0);

	g_xyTitleName[0].x = g_xywhMainBoard.x + ScreenHeight * (82 / 600.0);
	g_xyTitleName[0].y = g_xywhMainBoard.y + ScreenHeight * (20 / 600.0);

	g_xyTitleScore[0].x = g_xywhMainBoard.x + ScreenHeight * ((274 - 48) / 600.0);
	g_xyTitleScore[0].y = g_xyTitleName[0].y;

	g_xyTitleDeath[0].x = g_xywhMainBoard.x + ScreenHeight * (274 / 600.0);
	g_xyTitleDeath[0].y = g_xyTitleName[0].y;

	g_xyTitlePing[0].x = g_xywhMainBoard.x + ScreenHeight * (331 / 600.0);
	g_xyTitlePing[0].y = g_xyTitleName[0].y;

	g_xyTitleName[1].x = g_xywhMainBoard.x + ScreenHeight * (482 / 600.0);
	g_xyTitleName[1].y = g_xyTitleName[0].y;

	g_xyTitleScore[1].x = g_xywhMainBoard.x + ScreenHeight * ((674 - 48) / 600.0);
	g_xyTitleScore[1].y = g_xyTitleName[0].y;

	g_xyTitleDeath[1].x = g_xywhMainBoard.x + ScreenHeight * (674 / 600.0);
	g_xyTitleDeath[1].y = g_xyTitleName[0].y;

	g_xyTitlePing[1].x = g_xywhMainBoard.x + ScreenHeight * (732 / 600.0);
	g_xyTitlePing[1].y = g_xyTitleName[0].y;

	g_xyBoardLine[0].x = g_xywhMainBoard.x + ScreenHeight * (8 / 600.0);
	g_xyBoardLine[0].y = g_xywhMainBoard.y + ScreenHeight * (34 / 600.0);
	g_xyBoardLine[0].w = ScreenHeight * (374 / 600.0);
	g_xyBoardLine[0].h = 1;

	g_xyBoardLine[1].x = g_xywhMainBoard.x + ScreenHeight * (408 / 600.0);
	g_xyBoardLine[1].y = g_xyBoardLine[0].y;
	g_xyBoardLine[1].w = g_xyBoardLine[0].w;
	g_xyBoardLine[1].h = 1;

	g_xyBoardLine[2].x = g_xywhMainBoard.x + ScreenHeight * (394 / 600.0);
	g_xyBoardLine[2].y = g_xywhMainBoard.y + ScreenHeight * (8 / 600.0);
	g_xyBoardLine[2].w = 2;
	g_xyBoardLine[2].h = ScreenHeight * (365 / 600.0);

	//bottom board
	g_xywhBottomBoard.x = g_xywhMainBoard.x + ScreenHeight * (33 / 600.0);
	g_xywhBottomBoard.y = g_xywhMainBoard.y + ScreenHeight * (386 / 600.0);
	g_xywhBottomBoard.w = ScreenHeight * (725 / 600.0);
	g_xywhBottomBoard.h = ScreenHeight * (87 / 600.0);

	g_xyBoardLine[3].x = g_xywhBottomBoard.x + ScreenHeight * (106 / 600.0);
	g_xyBoardLine[3].y = g_xywhBottomBoard.y + ScreenHeight * (31 / 600.0);
	g_xyBoardLine[3].w = ScreenHeight * (581 / 600.0);
	g_xyBoardLine[3].h = 1;

	g_xywhBottomClassIcon.x = g_xywhMainBoard.x + ScreenHeight * (27 / 600.0);
	g_xywhBottomClassIcon.y = g_xywhMainBoard.y + ScreenHeight * (358 / 600.0);
	g_xywhBottomClassIcon.w = ScreenHeight * (116 / 600.0);
	g_xywhBottomClassIcon.h = g_xywhBottomClassIcon.w;

	g_xyBottomPlayerName.x = g_xywhBottomBoard.x + ScreenHeight * (105 / 600.0);
	g_xyBottomPlayerName.y = g_xywhBottomBoard.y + ScreenHeight * (7 / 600.0);

	g_xySpectators.x = g_xyBottomPlayerName.x;
	g_xySpectators.y = g_xywhBottomBoard.y - 15;

	for(int i = 0; i < STATS_MAX_NUM-1; ++i)
	{
		g_xyPlayerStats[i].x = g_xywhBottomBoard.x + ScreenHeight * ((198 + (i / 4) * 156) / 600.0);
		g_xyPlayerStats[i].y = g_xywhBottomBoard.y + ScreenHeight * (35 / 600.0) + (12 + ScreenHeight * (2 / 600.0) ) * (i % 4);
	}

	//fonts
	g_hFontTeamName = g_pSurface->CreateFont();
	g_pSurface->AddGlyphSetToFont(g_hFontTeamName, "TF2", 48 * ScreenHeight / 900, 0, 0, 0, vgui::ISurface::FONTFLAG_CUSTOM | vgui::ISurface::FONTFLAG_ANTIALIAS, 0x0, 0xFFFF);

	g_hFontPlayerNum = g_pSurface->CreateFont();
	g_pSurface->AddGlyphSetToFont(g_hFontPlayerNum, "TF2 Secondary", 40 * ScreenHeight / 900, 0, 0, 0, vgui::ISurface::FONTFLAG_CUSTOM | vgui::ISurface::FONTFLAG_ANTIALIAS, 0x0, 0xFFFF);
	
	g_hFontBottomPlayerName = g_pSurface->CreateFont();
	g_pSurface->AddGlyphSetToFont(g_hFontBottomPlayerName, "TF2 Secondary", 35 * ScreenHeight / 900, 0, 0, 0, vgui::ISurface::FONTFLAG_CUSTOM | vgui::ISurface::FONTFLAG_ANTIALIAS, 0x0, 0xFFFF);

	g_hFontScore = g_pSurface->CreateFont();
	g_pSurface->AddGlyphSetToFont(g_hFontScore, "TF2", 32 * ScreenHeight / 900, 0, 0, 0, vgui::ISurface::FONTFLAG_CUSTOM | vgui::ISurface::FONTFLAG_ANTIALIAS, 0x0, 0xFFFF);

	g_hFontTeamScore = g_pSurface->CreateFont();
	g_pSurface->AddGlyphSetToFont(g_hFontTeamScore, "TF2", 78 * ScreenHeight / 600, 0, 0, 0, vgui::ISurface::FONTFLAG_CUSTOM | vgui::ISurface::FONTFLAG_ANTIALIAS, 0x0, 0xFFFF);

	return 1;
}

void HudScoreBoard_Init(void)
{
	g_ScoreBoardEnabled = 0;
}

void DrawTeamBoard(void)
{
	wchar_t wszTemp[32];
	wchar_t wszTemp2[16];

	g_pSurface->DrawSetColor(255, 255, 255, 233);
	DrawHudMask(2, g_xywhBlueBoard.x, g_xywhBlueBoard.y, g_xywhBlueBoard.w, g_xywhBlueBoard.h);

	g_pSurface->DrawSetColor(255, 255, 255, 233);
	DrawHudMask(1, g_xywhRedBoard.x, g_xywhRedBoard.y, g_xywhRedBoard.w, g_xywhRedBoard.h);

	g_pSurface->DrawSetTextColor(g_ubColor[0].r, g_ubColor[0].g, g_ubColor[0].b, 255);

	//blue teamname
	g_pSurface->DrawSetTextPos(g_xyBlueTeam.x, g_xyBlueTeam.y);
	g_pSurface->DrawSetTextFont(g_hFontTeamName);
	g_pSurface->DrawPrintText(g_wszScoreBoardBlueTeam, wcslen(g_wszScoreBoardBlueTeam));

	//red teamname
	g_pSurface->DrawSetTextPos(g_xyRedTeam.x, g_xyRedTeam.y);
	g_pSurface->DrawSetTextFont(g_hFontTeamName);
	g_pSurface->DrawPrintText(g_wszScoreBoardRedTeam, wcslen(g_wszScoreBoardRedTeam));

	g_pSurface->DrawFlushText();

	//blue player num
	g_pSurface->DrawSetTextFont(g_hFontPlayerNum);
	
	g_pSurface->DrawSetTextPos(g_xyBluePlayerNum.x, g_xyBluePlayerNum.y);
	wsprintfW(wszTemp2, L"%d", g_iBluePlayerNum);
	g_pVGuiLocalize->ConstructString(wszTemp, sizeof(wszTemp), (g_iBluePlayerNum == 1) ? g_wszScoreBoardPlayerNum : g_wszScoreBoardPlayersNum, 1, wszTemp2);
	g_pSurface->DrawPrintText(wszTemp, wcslen(wszTemp));

	//red player num
	g_pSurface->DrawSetTextPos(g_xyRedPlayerNum.x, g_xyRedPlayerNum.y);
	wsprintfW(wszTemp2, L"%d", g_iRedPlayerNum);
	g_pVGuiLocalize->ConstructString(wszTemp, sizeof(wszTemp), (g_iRedPlayerNum == 1) ? g_wszScoreBoardPlayerNum : g_wszScoreBoardPlayersNum, 1, wszTemp2);
	g_pSurface->DrawPrintText(wszTemp, wcslen(wszTemp));

	g_pSurface->DrawFlushText();

	//blue team score
	g_pSurface->DrawSetTextFont(g_hFontTeamScore);

	wsprintfW(wszTemp, L"%d", g_iBlueTeamScore);
	g_pSurface->DrawSetTextColor(60, 57, 52, 255);
	g_pSurface->DrawSetTextPos(g_xyBlueTeamScore.x+1, g_xyBlueTeamScore.y+1);
	g_pSurface->DrawPrintText(wszTemp, wcslen(wszTemp));
	g_pSurface->DrawFlushText();

	g_pSurface->DrawSetTextColor(g_ubColor[0].r, g_ubColor[0].g, g_ubColor[0].b, 255);
	g_pSurface->DrawSetTextPos(g_xyBlueTeamScore.x, g_xyBlueTeamScore.y);
	g_pSurface->DrawPrintText(wszTemp, wcslen(wszTemp));
	g_pSurface->DrawFlushText();

	//red team score
	wsprintfW(wszTemp, L"%d", g_iRedTeamScore);
	g_pSurface->DrawSetTextColor(60, 57, 52, 255);
	g_pSurface->DrawSetTextPos(g_xyRedTeamScore.x+1, g_xyRedTeamScore.y+1);	
	g_pSurface->DrawPrintText(wszTemp, wcslen(wszTemp));
	g_pSurface->DrawFlushText();

	g_pSurface->DrawSetTextColor(g_ubColor[0].r, g_ubColor[0].g, g_ubColor[0].b, 255);
	g_pSurface->DrawSetTextPos(g_xyRedTeamScore.x, g_xyRedTeamScore.y);
	g_pSurface->DrawPrintText(wszTemp, wcslen(wszTemp));
	g_pSurface->DrawFlushText();
}

void DrawPlayers(int iTeam)
{
	char szTemp[16];
	if(iTeam == 1 || iTeam == 2)
	{
		int iWidth = ScreenHeight * (377 / 600.0);
		int iTotalHeight = ScreenHeight * (324 / 600.0);
		int iHeightOffset = ScreenHeight * (8 / 600.0);
		int iMinHeight = ScreenHeight * (24 / 600.0);
		int iMaxHeight = ScreenHeight * (27 / 600.0);
		int iTeamPlayerNum = (iTeam == 1) ? g_iRedPlayerNum : g_iBluePlayerNum;
		int iHeight = min(max(iTotalHeight / max(iTeamPlayerNum, 1), iMinHeight), iMaxHeight);

		xy_t *pos = (iTeam == 1) ? &g_xyRedPlayerInfo : &g_xyBluePlayerInfo;
		int *iPlayerIndexs = (iTeam == 1) ? &g_iRedPlayerIndexs[0] : &g_iBluePlayerIndexs[0];

		for(int i = 0; i < iTeamPlayerNum; ++i)
		{
			int index = iPlayerIndexs[i];

			if(g_HudPlayerInfo[index].thisplayer)
			{
				g_pSurface->DrawSetColor(255, 255, 255, 50);
				g_pSurface->DrawFilledRect(pos->x, pos->y, pos->x+iWidth, pos->y+iHeight);
			}

			int y = pos->y + iHeightOffset + i * iHeight;

			gEngfuncs.pfnDrawSetTextColor(1, 1, 1);
			gEngfuncs.pfnDrawConsoleString(g_xyTitleName[2-iTeam].x, y, g_HudPlayerInfo[index].name);

			int length, height;
			sprintf(szTemp, "%d", g_PlayerInfo[index].iFrags);
			gEngfuncs.pfnDrawConsoleStringLen(szTemp, &length, &height);
			gEngfuncs.pfnDrawSetTextColor(1, 1, 1);
			gEngfuncs.pfnDrawConsoleString(g_xyTitleScore[2-iTeam].x + 12 - length/2, y, szTemp);

			sprintf(szTemp, "%d", g_PlayerInfo[index].iDeaths);
			gEngfuncs.pfnDrawConsoleStringLen(szTemp, &length, &height);
			gEngfuncs.pfnDrawSetTextColor(1, 1, 1);
			gEngfuncs.pfnDrawConsoleString(g_xyTitleDeath[2-iTeam].x + 12 - length/2, y, szTemp);

			sprintf(szTemp, "%d", g_HudPlayerInfo[index].ping);
			gEngfuncs.pfnDrawConsoleStringLen(szTemp, &length, &height);
			gEngfuncs.pfnDrawSetTextColor(1, 1, 1);
			gEngfuncs.pfnDrawConsoleString(g_xyTitlePing[2-iTeam].x + 12 - length/2, y, szTemp);

			qglEnable(GL_BLEND);
		}
	}
}

void DrawMainBoard(void)
{
	//main board
	g_pSurface->DrawSetColor(41, 41, 41, 180);
	DrawHudMask(0, g_xywhMainBoard.x, g_xywhMainBoard.y, g_xywhMainBoard.w, g_xywhMainBoard.h);

	//main board line
	g_pSurface->DrawSetColor(242, 242, 242, 180);
	g_pSurface->DrawFilledRect(g_xyBoardLine[0].x, g_xyBoardLine[0].y, g_xyBoardLine[0].x+g_xyBoardLine[0].w, g_xyBoardLine[0].y+g_xyBoardLine[0].h);
	g_pSurface->DrawFilledRect(g_xyBoardLine[1].x, g_xyBoardLine[1].y, g_xyBoardLine[1].x+g_xyBoardLine[1].w, g_xyBoardLine[1].y+g_xyBoardLine[1].h);

	//main board mid line
	g_pSurface->DrawSetColor(28, 28, 28, 180);
	g_pSurface->DrawFilledRect(g_xyBoardLine[2].x, g_xyBoardLine[2].y, g_xyBoardLine[2].x+g_xyBoardLine[2].w, g_xyBoardLine[2].y+g_xyBoardLine[2].h);

	//main board blue players
	DrawPlayers(2);

	//main board red players
	DrawPlayers(1);

	//host name
	char szTemp[256];
	char szMapName[64];
	strcpy(szMapName, gEngfuncs.pfnGetLevelName() + 5);
	szMapName[strlen(szMapName)-4] = 0;

	int length, tall;
	sprintf(szTemp, "%s%s", g_szScoreBoardServerName, g_szServerName );
	gEngfuncs.pfnDrawSetTextColor(g_ubColor[0].r / 255.0, g_ubColor[0].g / 255.0, g_ubColor[0].b / 255.0);
	gEngfuncs.pfnDrawConsoleString(g_xyServerName.x, g_xyServerName.y, szTemp);

	sprintf(szTemp, "%s%s", g_szScoreBoardCurrentMap, szMapName );
	gEngfuncs.pfnDrawConsoleStringLen(szTemp, &length, &tall);
	gEngfuncs.pfnDrawSetTextColor(g_ubColor[0].r / 255.0, g_ubColor[0].g / 255.0, g_ubColor[0].b / 255.0);
	gEngfuncs.pfnDrawConsoleString(g_xyMapName.x - length, g_xyMapName.y, szTemp);

	//main board text
	gEngfuncs.pfnDrawSetTextColor(1, 1, 1)	;
	gEngfuncs.pfnDrawConsoleString(g_xyTitleName[0].x, g_xyTitleName[0].y, g_szScoreBoardName);
	gEngfuncs.pfnDrawSetTextColor(1, 1, 1);
	gEngfuncs.pfnDrawConsoleString(g_xyTitleName[1].x, g_xyTitleName[1].y, g_szScoreBoardName);

	gEngfuncs.pfnDrawSetTextColor(1, 1, 1);
	gEngfuncs.pfnDrawConsoleString(g_xyTitleScore[0].x, g_xyTitleScore[0].y, g_szScoreBoardScore);
	gEngfuncs.pfnDrawSetTextColor(1, 1, 1);
	gEngfuncs.pfnDrawConsoleString(g_xyTitleScore[1].x, g_xyTitleScore[1].y, g_szScoreBoardScore);

	gEngfuncs.pfnDrawSetTextColor(1, 1, 1);
	gEngfuncs.pfnDrawConsoleString(g_xyTitleDeath[0].x, g_xyTitleDeath[0].y, g_szScoreBoardDeath);
	gEngfuncs.pfnDrawSetTextColor(1, 1, 1);
	gEngfuncs.pfnDrawConsoleString(g_xyTitleDeath[1].x, g_xyTitleDeath[1].y, g_szScoreBoardDeath);

	gEngfuncs.pfnDrawSetTextColor(1, 1, 1);
	gEngfuncs.pfnDrawConsoleString(g_xyTitlePing[0].x, g_xyTitlePing[0].y, g_szScoreBoardPing);
	gEngfuncs.pfnDrawSetTextColor(1, 1, 1);
	gEngfuncs.pfnDrawConsoleString(g_xyTitlePing[1].x, g_xyTitlePing[1].y, g_szScoreBoardPing);

	qglEnable(GL_BLEND);
}

int GetPlayerStatsInfo(int index)
{
	switch(index)
	{
	case STATS_DEMOLISH:
		return g_PlayerStatsInfo.iDemolish;
		break;
	case STATS_CAPTURE:
		return g_PlayerStatsInfo.iCapture;
		break;
	case STATS_DEFENCE:
		return g_PlayerStatsInfo.iDefence;
		break;
	case STATS_DOMINATE:
		return g_PlayerStatsInfo.iDominate;
		break;
	case STATS_REVENGE:
		return g_PlayerStatsInfo.iRevenge;
		break;
	case STATS_UBERCHARGE:
		return g_PlayerStatsInfo.iUbercharge;
		break;
	case STATS_HEADSHOT:
		return g_PlayerStatsInfo.iHeadshot;
		break;
	case STATS_TELEPORT:
		return g_PlayerStatsInfo.iTeleport;
		break;
	case STATS_HEALING:
		return g_PlayerStatsInfo.iHealing;
		break;
	case STATS_BACKSTAB:
		return g_PlayerStatsInfo.iBackstab;
		break;
	case STATS_BONUS:
		return g_PlayerStatsInfo.iBonus;
		break;
	case STATS_KILL:
		return g_PlayerStatsInfo.iKill;
		break;
	case STATS_DEATH:
		return g_PlayerStatsInfo.iDeath;
		break;
	case STATS_ASSIST:
		return g_PlayerStatsInfo.iAssist;
		break;
	}
	return 0;
}

void DrawBottomBoard(void)
{
	wchar_t wszTemp[64];

	//bottom board
	g_pSurface->DrawSetColor(28, 28, 28, 180);
	g_pSurface->DrawFilledRect(g_xywhBottomBoard.x, g_xywhBottomBoard.y, g_xywhBottomBoard.x+g_xywhBottomBoard.w, g_xywhBottomBoard.y+g_xywhBottomBoard.h);

	if(g_iTeam == 1 || g_iTeam == 2)
	{
		g_pSurface->DrawSetColor(255, 255, 255, 255);
		g_pSurface->DrawSetTexture(g_texPlayerMan[g_iTeam-1][g_iClass-1]);
		g_pSurface->DrawTexturedRect(g_xywhBottomClassIcon.x, g_xywhBottomClassIcon.y, g_xywhBottomClassIcon.x+g_xywhBottomClassIcon.w, g_xywhBottomClassIcon.y+g_xywhBottomClassIcon.h);
	}

	//bottom line
	int iColor = (g_iTeam > 2) ? 0 : g_iTeam;

	g_pSurface->DrawSetColor(g_ubColor[iColor].r, g_ubColor[iColor].g, g_ubColor[iColor].b, 180);
	g_pSurface->DrawFilledRect(g_xyBoardLine[3].x, g_xyBoardLine[3].y, g_xyBoardLine[3].x+g_xyBoardLine[3].w, g_xyBoardLine[3].y+g_xyBoardLine[3].h);

	//player name
	wchar_t *pwszPlayerName = UTF8ToUnicode(g_HudPlayerInfo[g_iLocalPlayerNum].name);
	g_pSurface->DrawSetTextFont(g_hFontBottomPlayerName);
	g_pSurface->DrawSetTextColor(g_ubColor[iColor].r, g_ubColor[iColor].g, g_ubColor[iColor].b, 255);
	g_pSurface->DrawSetTextPos(g_xyBottomPlayerName.x, g_xyBottomPlayerName.y);
	g_pSurface->DrawPrintText(pwszPlayerName, wcslen(pwszPlayerName));
	g_pSurface->DrawFlushText();

	//bottom board text

	//spectators

	gEngfuncs.pfnDrawSetTextColor(1, 1, 1);
	std::string buf = g_szScoreBoardSpectator;
	for(int i = 0; i < g_iSpectatorNum; ++i)
	{
		if(i != 0) buf.append(", ");
		buf.append(g_HudPlayerInfo[g_iSpectatorIndexs[i]].name);		
	}
	gEngfuncs.pfnDrawSetTextColor(1, 1, 1);
	gEngfuncs.pfnDrawConsoleString(g_xySpectators.x, g_xySpectators.y, (char *)buf.c_str() );
 
	//stats info
	int w, h;
	for(int i = 1; i < STATS_MAX_NUM; ++i)
	{
		wsprintfW(wszTemp, L"%s%d", g_wszPlayerStatsName[i-1], GetPlayerStatsInfo(i) );
		char *pszTemp = UnicodeToUTF8(wszTemp);
		gEngfuncs.pfnDrawConsoleStringLen(pszTemp, &w, &h);
		gEngfuncs.pfnDrawSetTextColor(1, 1, 1);
		gEngfuncs.pfnDrawConsoleString(g_xyPlayerStats[i-1].x - w, g_xyPlayerStats[i-1].y, pszTemp);
	}
	qglEnable(GL_BLEND);
}

void GetPlayerInfo(void)
{
	g_iRedPlayerNum = 0;
	g_iBluePlayerNum = 0;
	g_iSpectatorNum = 0;
	for(int i = 1; i < gEngfuncs.GetMaxClients(); ++i)
	{
		gEngfuncs.pfnGetPlayerInfo(i, &g_HudPlayerInfo[i]);

		if(g_HudPlayerInfo[i].thisplayer)
			g_iLocalPlayerNum = i;

		if(g_HudPlayerInfo[i].name && g_HudPlayerInfo[i].name[0])
		{
			if(g_PlayerInfo[i].iTeam == 1)
			{
				g_iRedPlayerIndexs[g_iRedPlayerNum] = i;
				g_iRedPlayerNum ++;				
			}
			else if(g_PlayerInfo[i].iTeam == 2)
			{
				g_iBluePlayerIndexs[g_iBluePlayerNum] = i;
				g_iBluePlayerNum ++;
			}
			else
			{
				g_iSpectatorIndexs[g_iSpectatorNum] = i;
				g_iSpectatorNum ++;
			}
		}
	}
}

int fnSortPlayers(const void *a, const void *b)
{
	int iPlayerA = *(int *)a;
	int iPlayerB = *(int *)b;
	return g_PlayerInfo[iPlayerA].iFrags - g_PlayerInfo[iPlayerB].iFrags;
}

void SortPlayers(void)
{
	if(g_iRedPlayerNum)
	{
		qsort(g_iRedPlayerIndexs, g_iRedPlayerNum, sizeof(int), fnSortPlayers);
	}
	if(g_iBluePlayerNum)
	{
		qsort(g_iBluePlayerIndexs, g_iBluePlayerNum, sizeof(int), fnSortPlayers);
	}
}

int HudScoreBoard_Redraw(float flTime, int iIntermission)
{
	GetPlayerInfo();
	SortPlayers( );
	DrawTeamBoard();
	DrawMainBoard();
	DrawBottomBoard();

	return 1;
}