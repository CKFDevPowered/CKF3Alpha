#include "hud_base.h"
#include "exportfuncs.h"
#include "client.h"
#include "util.h"

wchar_t g_wszExit[16];
wchar_t g_wszNext[16];
wchar_t g_wszPrev[16];
wchar_t g_wszChooseClass[16];
wchar_t g_wszClassify[3][16];
wchar_t g_wszChooseTeam[16];
wchar_t g_wszTeamMenu_Spect[16];
wchar_t g_wszBuild[16];
wchar_t g_wszDemolish[16];
wchar_t g_wszDisguise[16];
wchar_t g_wszRandom[16];

wchar_t g_wszBuildables[5][64];
char g_szBuildables[5][64];

wchar_t g_wszNotBuilt[32];
char g_szNotBuilt[32];
wchar_t g_wszBuilding[32];
char g_szAlreadyBuilt[32];
char g_szNeedMetal[32];
char g_szHudMenuBack[64];
char g_szDisgMenuSwitch[64];
wchar_t g_wszCloakEnergy[64];

char g_szScoreBoardSpectator[32];
char g_szScoreBoardName[16];
char g_szScoreBoardScore[16];
char g_szScoreBoardDeath[16];
char g_szScoreBoardAssist[16];
char g_szScoreBoardPing[16];
char g_szScoreBoardServerName[64];
char g_szScoreBoardCurrentMap[64];
wchar_t g_wszScoreBoardPlayerNum[32];
wchar_t g_wszScoreBoardPlayersNum[32];
wchar_t g_wszScoreBoardRedTeam[12];
wchar_t g_wszScoreBoardBlueTeam[12];
wchar_t g_wszPlayerStatsName[STATS_MAX_NUM-1][16];

wchar_t g_wszClassTips[10][4][64];
wchar_t g_wszRoundStatus[4][16];

char g_szMsgCaptured[16];
char g_szMsgDefended[16];
char g_szMsgDominating[16];
char g_szMsgRevenge[16];

namespace vgui
{
	wchar_t *ConvertVGUIKeyCodeToUnicodeString(vgui::KeyCode code);
};

int HudLocalize_VidInit(void)
{
	_LocalizeStringUnicode(g_wszExit, "#CKF3_Menu_Exit");
	_LocalizeStringUnicode(g_wszNext, "#CKF3_Menu_Next");
	_LocalizeStringUnicode(g_wszPrev, "#CKF3_Menu_Prev");
	_LocalizeStringUnicode(g_wszChooseClass, "#CKF3_ClassMenu_ChooseClass");
	_LocalizeStringUnicode(g_wszClassify[0], "#CKF3_ClassMenu_Attack");
	_LocalizeStringUnicode(g_wszClassify[1], "#CKF3_ClassMenu_Defence");
	_LocalizeStringUnicode(g_wszClassify[2], "#CKF3_ClassMenu_Support");	
	_LocalizeStringUnicode(g_wszChooseTeam, "#CKF3_TeamMenu_ChooseTeam");
	_LocalizeStringUnicode(g_wszTeamMenu_Spect, "#CKF3_TeamMenu_Spect");
	_LocalizeStringUnicode(g_wszCloakEnergy, "#CKF3_CloakEnergy");
	_LocalizeStringAll(g_szBuildables[0], g_wszBuildables[0], "#CKF3_Object_Sentry");
	_LocalizeStringAll(g_szBuildables[1], g_wszBuildables[1], "#CKF3_Object_Dispenser");
	_LocalizeStringAll(g_szBuildables[2], g_wszBuildables[2], "#CKF3_Object_Tele_Entrance");
	_LocalizeStringAll(g_szBuildables[3], g_wszBuildables[3], "#CKF3_Object_Tele_Exit");
	_LocalizeStringAll(g_szBuildables[4], g_wszBuildables[4], "#CKF3_Object_Sapper");
	_LocalizeStringUnicode(g_wszBuilding, "#CKF3_Buildable_Building");
	_LocalizeStringAll(g_szNotBuilt, g_wszNotBuilt, "#CKF3_Buildable_NotBuilt");
	_LocalizeStringUTF8(g_szAlreadyBuilt, "#CKF3_BuildMenu_AlreadyBuilt");
	_LocalizeStringUTF8(g_szNeedMetal, "#CKF3_BuildMenu_NeedMetal");
	_LocalizeStringUnicode(g_wszBuild, "#CKF3_Build");
	_LocalizeStringUnicode(g_wszDemolish, "#CKF3_Demolish");
	_LocalizeStringUnicode(g_wszDisguise,  "#CKF3_Disguise");
	_LocalizeStringUnicode(g_wszRandom, "#CKF3_Random");
	_LocalizeStringUnicode(g_wszPlayerStatsName[STATS_KILL-1], "#CKF3_Stats_Kill");
	_LocalizeStringUnicode(g_wszPlayerStatsName[STATS_DEATH-1], "#CKF3_Stats_Death");
	_LocalizeStringUnicode(g_wszPlayerStatsName[STATS_ASSIST-1], "#CKF3_Stats_Assist");
	_LocalizeStringUnicode(g_wszPlayerStatsName[STATS_DEMOLISH-1], "#CKF3_Stats_Demolish");
	_LocalizeStringUnicode(g_wszPlayerStatsName[STATS_CAPTURE-1], "#CKF3_Stats_Capture");
	_LocalizeStringUnicode(g_wszPlayerStatsName[STATS_DEFENCE-1], "#CKF3_Stats_Defence");
	_LocalizeStringUnicode(g_wszPlayerStatsName[STATS_DOMINATE-1], "#CKF3_Stats_Dominate");
	_LocalizeStringUnicode(g_wszPlayerStatsName[STATS_DEFENCE-1], "#CKF3_Stats_Defence");
	_LocalizeStringUnicode(g_wszPlayerStatsName[STATS_REVENGE-1], "#CKF3_Stats_Revenge");
	_LocalizeStringUnicode(g_wszPlayerStatsName[STATS_UBERCHARGE-1], "#CKF3_Stats_Ubercharge");
	_LocalizeStringUnicode(g_wszPlayerStatsName[STATS_HEADSHOT-1], "#CKF3_Stats_Headshot");
	_LocalizeStringUnicode(g_wszPlayerStatsName[STATS_TELEPORT-1], "#CKF3_Stats_Teleport");
	_LocalizeStringUnicode(g_wszPlayerStatsName[STATS_HEALING-1], "#CKF3_Stats_Healing");
	_LocalizeStringUnicode(g_wszPlayerStatsName[STATS_BACKSTAB-1], "#CKF3_Stats_Backstab");
	_LocalizeStringUnicode(g_wszPlayerStatsName[STATS_BONUS-1], "#CKF3_Stats_Bonus");
	_LocalizeStringUTF8(g_szScoreBoardSpectator, "#CKF3_ScoreBoard_Spectator");
	_LocalizeStringUTF8(g_szScoreBoardName, "#CKF3_ScoreBoard_Name");
	_LocalizeStringUTF8(g_szScoreBoardScore, "#CKF3_ScoreBoard_Score");
	_LocalizeStringUTF8(g_szScoreBoardDeath, "#CKF3_ScoreBoard_Death");
	_LocalizeStringUTF8(g_szScoreBoardAssist, "#CKF3_ScoreBoard_Assist");
	_LocalizeStringUTF8(g_szScoreBoardPing, "#CKF3_ScoreBoard_Ping");
	_LocalizeStringUTF8(g_szScoreBoardServerName, "#CKF3_ScoreBoard_ServerName");
	_LocalizeStringUTF8(g_szScoreBoardCurrentMap, "#CKF3_ScoreBoard_CurrentMap");
	_LocalizeStringUnicode(g_wszScoreBoardPlayerNum, "#CKF3_ScoreBoard_PlayerNum");
	_LocalizeStringUnicode(g_wszScoreBoardPlayersNum, "#CKF3_ScoreBoard_PlayersNum");
	_LocalizeStringUnicode(g_wszScoreBoardRedTeam, "#CKF3_ScoreBoard_RedTeam");
	_LocalizeStringUnicode(g_wszScoreBoardBlueTeam, "#CKF3_ScoreBoard_BlueTeam");

	memset(g_wszClassTips, 0, sizeof(g_wszClassTips));
	_LocalizeStringUnicode(g_wszClassTips[0][0], "#CKF3_ClassTips_Scout_1");
	_LocalizeStringUnicode(g_wszClassTips[0][1], "#CKF3_ClassTips_Scout_2");
	_LocalizeStringUnicode(g_wszClassTips[1][0], "#CKF3_ClassTips_Soldier_1");
	_LocalizeStringUnicode(g_wszClassTips[1][1], "#CKF3_ClassTips_Soldier_2");
	_LocalizeStringUnicode(g_wszClassTips[2][0], "#CKF3_ClassTips_Pyro_1");
	_LocalizeStringUnicode(g_wszClassTips[2][1], "#CKF3_ClassTips_Pyro_2");
	_LocalizeStringUnicode(g_wszClassTips[3][0], "#CKF3_ClassTips_Demoman_1");
	_LocalizeStringUnicode(g_wszClassTips[3][1], "#CKF3_ClassTips_Demoman_2");
	_LocalizeStringUnicode(g_wszClassTips[4][0], "#CKF3_ClassTips_Heavy_1");
	_LocalizeStringUnicode(g_wszClassTips[5][0], "#CKF3_ClassTips_Engineer_1");
	_LocalizeStringUnicode(g_wszClassTips[5][1], "#CKF3_ClassTips_Engineer_2");
	_LocalizeStringUnicode(g_wszClassTips[5][2], "#CKF3_ClassTips_Engineer_3");
	_LocalizeStringUnicode(g_wszClassTips[6][0], "#CKF3_ClassTips_Medic_1");
	_LocalizeStringUnicode(g_wszClassTips[6][1], "#CKF3_ClassTips_Medic_2");
	_LocalizeStringUnicode(g_wszClassTips[7][0], "#CKF3_ClassTips_Sniper_1");
	_LocalizeStringUnicode(g_wszClassTips[7][1], "#CKF3_ClassTips_Sniper_2");
	_LocalizeStringUnicode(g_wszClassTips[8][0], "#CKF3_ClassTips_Spy_1");
	_LocalizeStringUnicode(g_wszClassTips[8][1], "#CKF3_ClassTips_Spy_2");
	_LocalizeStringUnicode(g_wszClassTips[8][2], "#CKF3_ClassTips_Spy_3");
	_LocalizeStringUnicode(g_wszClassTips[8][3], "#CKF3_ClassTips_Spy_4");
	_LocalizeStringUnicode(g_wszClassTips[9][0], "#CKF3_ClassTips_Random_1");

	_LocalizeStringUnicode(g_wszRoundStatus[0], "#CKF3_RoundStatus_Setup");
	_LocalizeStringUnicode(g_wszRoundStatus[1], "#CKF3_RoundStatus_End");
	_LocalizeStringUnicode(g_wszRoundStatus[2], "#CKF3_RoundStatus_Overtime");
	_LocalizeStringUnicode(g_wszRoundStatus[3], "#CKF3_RoundStatus_Wait");

	_LocalizeStringUTF8(g_szMsgCaptured, "#Msg_Captured");
	_LocalizeStringUTF8(g_szMsgDefended, "#Msg_Defended");
	_LocalizeStringUTF8(g_szMsgDominating, "#Msg_Dominating");
	_LocalizeStringUTF8(g_szMsgRevenge, "#Msg_Revenge");

	wchar_t keyname[16], format[32], temp[32];
	vgui::KeyCode key;

	_LocalizeStringUnicode(format, "#CKF3_HudMenu_Back");
	key = g_pGameUIFuncs->GetVGUI2KeyCodeForBind("lastinv");
	
	wcsncpy(keyname, ConvertVGUIKeyCodeToUnicodeString(key), 15);
	keyname[15] = L'\0';
	
	g_pVGuiLocalize->ConstructString(temp, sizeof(temp), format, 1, keyname);
	strncpy(g_szHudMenuBack, UnicodeToUTF8(temp), sizeof(g_szHudMenuBack)-1);
	g_szHudMenuBack[sizeof(g_szHudMenuBack)-1] = 0;

	_LocalizeStringUnicode(format, "#CKF3_DisgMenu_SwitchTeam");
	key = g_pGameUIFuncs->GetVGUI2KeyCodeForBind("reload");
	
	wcsncpy(keyname, ConvertVGUIKeyCodeToUnicodeString(key), 15);
	keyname[15] = L'\0';

	g_pVGuiLocalize->ConstructString(temp, sizeof(temp), format, 1, keyname);
	strncpy(g_szDisgMenuSwitch, UnicodeToUTF8(temp), sizeof(g_szDisgMenuSwitch)-1);
	g_szDisgMenuSwitch[sizeof(g_szDisgMenuSwitch)-1] = 0;

	return 1;
}