#pragma once

#include "client.h"

extern wchar_t g_wszExit[16];
extern wchar_t g_wszNext[16];
extern wchar_t g_wszPrev[16];
extern wchar_t g_wszChooseClass[16];
extern wchar_t g_wszClassify[3][16];
extern wchar_t g_wszChooseTeam[16];
extern wchar_t g_wszTeamMenu_Spect[16];
extern wchar_t g_wszBuild[16];
extern wchar_t g_wszDemolish[16];
extern wchar_t g_wszDisguise[16];
extern wchar_t g_wszRandom[16];


extern wchar_t g_wszBuildables[4][64];
extern char g_szBuildables[4][64];
extern wchar_t g_wszNotBuilt[32];
extern char g_szNotBuilt[32];
extern wchar_t g_wszBuilding[32];
extern char g_szAlreadyBuilt[32];
extern char g_szNeedMetal[32];
extern char g_szHudMenuBack[64];
extern char g_szDisgMenuSwitch[64];

extern char g_szScoreBoardSpectator[32];
extern char g_szScoreBoardName[16];
extern char g_szScoreBoardScore[16];
extern char g_szScoreBoardDeath[16];
extern char g_szScoreBoardAssist[16];
extern char g_szScoreBoardPing[16];
extern char g_szScoreBoardServerName[64];
extern char g_szScoreBoardCurrentMap[64];
extern wchar_t g_wszScoreBoardPlayerNum[32];
extern wchar_t g_wszScoreBoardPlayersNum[32];
extern wchar_t g_wszScoreBoardRedTeam[12];
extern wchar_t g_wszScoreBoardBlueTeam[12];
extern wchar_t g_wszPlayerStatsName[STATS_MAX_NUM-1][16];

extern wchar_t g_wszClassTips[10][4][64];
extern wchar_t g_wszRoundStatus[4][16];

void LocalizeStringUTF8(char *inputString, int inputStringLength, const char *tokenString);
void LocalizeStringUnicode(wchar_t *inputStringW, int inputStringWLength, const char *tokenString);

#define _LocalizeStringUTF8(strBuf, token) LocalizeStringUTF8(strBuf, sizeof(strBuf), token);
#define _LocalizeStringUnicode(strBufW, token) LocalizeStringUnicode(strBufW, sizeof(strBufW)/2, token);
#define _LocalizeStringAll(strBuf, strBufW, token) {_LocalizeStringUTF8(strBuf, token);_LocalizeStringUnicode(strBufW, token);}